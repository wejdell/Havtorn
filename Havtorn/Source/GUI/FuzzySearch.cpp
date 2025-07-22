// Copyright 2025 Team Havtorn. All Rights Reserved.

/**************************************************************************/
/*  fuzzy_search.cpp                                                      */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include <queue>
#include <ranges>
#include "FuzzySearch.h"

namespace Havtorn
{
	constexpr float CullFactor = 0.25f;
	constexpr float CullCutOff = 10.0f;
	const std::string BoundryChars = "/\\-_.";

	static bool IsValidInterval(const SVector2<I32>& interval)
	{
		// Empty intervals are represented as (-1, -1).
		return interval.X >= 0 && interval.Y >= interval.X;
	}

	static SVector2<I32> ExtendedInterval(const SVector2<I32>& a, const SVector2<I32>& b)
	{
		if (!IsValidInterval(a))
		{
			return b;
		}
		if (!IsValidInterval(b))
		{
			return a;
		}
		return SVector2<I32>(UMath::Min(a.X, b.X), UMath::Max(a.Y, b.Y));
	}

	static bool IsWordBoundry(const std::string& str, I32 index)
	{
		if (index == -1 || index == str.size())
		{
			return true;
		}
		return BoundryChars.find(str[index]) != -1;
	}

	bool FuzzySearchToken::TryExactMatch(FuzzyTokenMatch& match, const std::string& target, int offset) const
	{
		match.TokenIndex = Index;
		match.TokenLength = STATIC_I32(String.length());
		int match_idx = STATIC_I32(target.find(String, offset));
		if (match_idx == -1)
		{
			return false;
		}
		match.AddSubString(match_idx, STATIC_I32(String.length()));
		return true;
	}

	bool FuzzySearchToken::TryFuzzyMatch(FuzzyTokenMatch& match, const std::string& target, int offset, int missBudget) const
	{
		match.TokenIndex = Index;
		match.TokenLength = STATIC_I32(String.length());
		I32 runStart = -1;
		I32 runLength = 0;

		// Search for the subsequence p_token in target starting from offset, recording each substring for
		// later scoring and display.
		for (I32 i = 0; i < String.length(); i++)
		{
			I32 newOffset = STATIC_I32(target.find(String[i], offset));
			if (newOffset < 0)
			{
				missBudget--;
				if (missBudget < 0)
				{
					return false;
				}
			}
			else
			{
				if (runStart == -1 || offset != newOffset)
				{
					if (runStart != -1)
					{
						match.AddSubString(runStart, runLength);
					}
					runStart = newOffset;
					runLength = 1;
				}
				else
				{
					runLength += 1;
				}
				offset = newOffset + 1;
			}
		}

		if (runStart != -1)
		{
			match.AddSubString(runStart, runLength);
		}

		return true;
	}

	void FuzzyTokenMatch::AddSubString(I32 subStringStart, I32 subStringLength)
	{
		SubStringIndexPairs.emplace_back(SVector2<U32>(subStringStart, subStringLength));
		MatchedLength += subStringLength;
		SVector2<I32> substring_interval = { subStringStart, subStringStart + subStringLength - 1 };
		Interval = ExtendedInterval(Interval, substring_interval);
	}

	bool FuzzyTokenMatch::Intersects(const SVector2<I32>& otherInterval) const
	{
		if (!IsValidInterval(Interval) || !IsValidInterval(otherInterval))
		{
			return false;
		}
		return Interval.Y >= otherInterval.X && Interval.X <= otherInterval.Y;
	}


	bool FuzzySearchResult::CanAddTokenMatch(const FuzzyTokenMatch& match) const
	{
		if (match.GetMissCount() > MissBudget)
		{
			return false;
		}

		if (match.Intersects(MatchInterval))
		{
			if (TokenMatches.size() == 1)
			{
				return false;
			}
			for (const FuzzyTokenMatch& existing_match : TokenMatches)
			{
				if (existing_match.Intersects(match.Interval))
				{
					return false;
				}
			}
		}

		return true;
	}

	bool FuzzyTokenMatch::IsCaseInsensitive(const String& original, const String& adjusted) const
	{
		for (const SVector2<U32>& subStringIndexPair : SubStringIndexPairs)
		{
			const int end = subStringIndexPair.X + subStringIndexPair.Y;
			for (int i = subStringIndexPair.X; i < end; i++)
			{
				if (original[i] != adjusted[i])
				{
					return true;
				}
			}
		}
		return false;
	}

	void FuzzySearchResult::ScoreTokenMatch(FuzzyTokenMatch& match, bool caseInsensitive) const
	{
		// This can always be tweaked more. The intuition is that exact matches should almost always
		// be prioritized over broken up matches, and other criteria more or less act as tie breakers.

		match.Score = -20 * match.GetMissCount() - (caseInsensitive ? 3 : 0);

		for (const SVector2<U32>& subStringIndexPair : match.SubStringIndexPairs)
		{
			// Score longer substrings higher than short substrings.
			int subStringIndexPairScore = subStringIndexPair.Y * subStringIndexPair.Y;
			// Score matches deeper in path higher than shallower matches
			if (STATIC_I32(subStringIndexPair.X) > DirIndex)
			{
				subStringIndexPairScore *= 2;
			}
			// Score matches on a word boundary higher than matches within a word
			if (IsWordBoundry(Target, STATIC_I32(subStringIndexPair.X - 1)) || IsWordBoundry(Target, subStringIndexPair.X + subStringIndexPair.Y))
			{
				subStringIndexPairScore += 4;
			}
			// Score exact query matches higher than non-compact subsequence matches
			if (subStringIndexPair.Y == match.TokenLength)
			{
				subStringIndexPairScore += 100;
			}

			match.Score += subStringIndexPairScore;
		}
	}

	void FuzzySearchResult::MaybeApplyScoreBonus()
	{
		// This adds a small bonus to results which match tokens in the same order they appear in the query.
		std::vector<int> TokenRangeStarts;
		TokenRangeStarts.resize(TokenMatches.size());

		for (const FuzzyTokenMatch& match : TokenMatches)
		{
			TokenRangeStarts[match.TokenIndex] = match.Interval.X;
		}

		int last = TokenRangeStarts[0];
		for (int i = 1; i < TokenMatches.size(); i++)
		{
			if (last > TokenRangeStarts[i])
			{
				return;
			}
			last = TokenRangeStarts[i];
		}

		Score += 1;
	}

	void FuzzySearchResult::AddTokenMatch(const FuzzyTokenMatch& match)
	{
		Score += match.Score;
		MatchInterval = ExtendedInterval(MatchInterval, match.Interval);
		MissBudget -= match.GetMissCount();
		TokenMatches.emplace_back(match);
	}

	void remove_low_scores(std::vector<FuzzySearchResult>& results, float cullScore)
	{
		// Removes all results with score < cullScore in-place.
		I32 i = 0;
		I32 j = STATIC_I32(results.size() - 1);

		while (true)
		{
			// Advances i to an element to remove and j to an element to keep.
			while (j >= i && results[j].Score < cullScore)
			{
				j--;
			}
			while (i < j && results[i].Score >= cullScore)
			{
				i++;
			}
			if (i >= j)
			{
				break;
			}

			results[i++] = results[j--];
		}

		results.resize(j + 1);
	}

	void FuzzySearch::SortAndFilter(std::vector<FuzzySearchResult>& results) const
	{
		if (results.empty())
		{
			return;
		}

		F32 averageScore = 0;
		F32 maxScore = 0;

		for (const FuzzySearchResult& result : results)
		{
			averageScore += result.Score;
			maxScore = UMath::Max(maxScore, STATIC_F32(result.Score));
		}

		// TODO: Tune scoring and culling here to display fewer subsequence soup matches when good matches
		//  are available.
		averageScore /= results.size();
		F32 cullScore = UMath::Min(CullCutOff, UMath::Lerp(averageScore, maxScore, CullFactor));
		remove_low_scores(results, cullScore);

		struct FuzzySearchResultComparator
		{
			bool operator()(const FuzzySearchResult& left, const FuzzySearchResult& right) const
			{
				// Sort on (score, length, alphanumeric) to ensure consistent ordering.
				if (left.Score == right.Score)
				{
					if (left.Target.length() == right.Target.length())
					{
						return left.Target < right.Target;
					}
					return left.Target.length() < right.Target.length();
				}
				return left.Score > right.Score;
			}
		};

		//TODO.AS 2025-07-22
		//If we want to partial sort at some point when we have Soo many Nodes or other things- This is where we' will want to do that.
		std::sort(results.begin(), results.end(), FuzzySearchResultComparator{ });
	}

	bool IsLower(const unsigned char c)
	{
		return std::islower(c);
	}

	void FuzzySearch::SetQuery(const String& query)
	{
		Tokens.clear();

		auto split = std::ranges::views::split(query, " ");
		for (auto it = split.begin(); it != split.end(); it++)
		{
			std::string splitString = (*it).data();
			Tokens.push_back({ STATIC_I32(Tokens.size()), splitString });
		}

		bool isLowerCase = true;
		for (auto& c : query)
		{
			if (!IsLower(c))
			{
				isLowerCase = false;
				break;
			}
		}

		CaseSensitive = !isLowerCase;

		struct TokenComparator
		{
			bool operator()(const FuzzySearchToken& a, const FuzzySearchToken& b) const
			{
				if (a.String.length() == b.String.length())
				{
					return a.Index < b.Index;
				}
				return a.String.length() > b.String.length();
			}
		};

		// Prioritize matching longer tokens before shorter ones since match overlaps are not accepted.
		std::sort<std::vector<FuzzySearchToken>::iterator>(Tokens.begin(), Tokens.end(), TokenComparator{});
	}

	bool FuzzySearch::Search(const String& target, FuzzySearchResult& result) const
	{
		result.Target = target;
		result.DirIndex = STATIC_I32(target.rfind('/'));
		result.MissBudget = MaxMisses;

		std::string adjustedTarget; // = case_sensitive ? target : target.to_lower();
		if (CaseSensitive)
		{
			adjustedTarget.resize(target.size());
			std::transform(target.begin(), target.end(), adjustedTarget.begin(), [](unsigned char c) { return std::tolower(c); });
		}
		else
		{
			adjustedTarget = target;
		}

		// For each token, eagerly generate subsequences starting from index 0 and keep the best scoring one
		// which does not conflict with prior token matches. This is not ensured to find the highest scoring
		// combination of matches, or necessarily the highest scoring single subsequence, as it only considers
		// eager subsequences for a given index, and likewise eagerly finds matches for each token in sequence.
		for (const FuzzySearchToken& token : Tokens)
		{
			FuzzyTokenMatch bestMatch;
			int offset = StartOffset;

			while (true)
			{
				FuzzyTokenMatch match;
				if (AllowSubsequences)
				{
					if (!token.TryFuzzyMatch(match, adjustedTarget, offset, result.MissBudget))
					{
						break;
					}
				}
				else
				{
					if (!token.TryExactMatch(match, adjustedTarget, offset))
					{
						break;
					}
				}
				if (result.CanAddTokenMatch(match))
				{
					result.ScoreTokenMatch(match, match.IsCaseInsensitive(target, adjustedTarget));
					if (bestMatch.TokenIndex == -1 || bestMatch.Score < match.Score)
					{
						bestMatch = match;
					}
				}
				if (IsValidInterval(match.Interval))
				{
					offset = match.Interval.X + 1;
				}
				else
				{
					break;
				}
			}

			if (bestMatch.TokenIndex == -1)
			{
				return false;
			}

			result.AddTokenMatch(bestMatch);
		}

		result.MaybeApplyScoreBonus();
		return true;
	}

	void FuzzySearch::SearchAll(const std::vector<std::string>& targets, std::vector<FuzzySearchResult>& results) const
	{
		results.clear();

		for (const String& target : targets)
		{
			FuzzySearchResult result;
			if (Search(target, result))
			{
				results.emplace_back(result);
			}
		}

		SortAndFilter(results);
	}
}