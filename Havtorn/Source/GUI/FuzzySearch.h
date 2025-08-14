// Copyright 2025 Team Havtorn. All Rights Reserved.

/**************************************************************************/
/*  fuzzy_search.h                                                        */
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
#pragma once

#include <string>
#include <MathTypes/Vector.h>

namespace Havtorn
{
	class FuzzyTokenMatch;
	struct FuzzySearchToken
	{
		int Index = -1;
		std::string String;

		bool TryExactMatch(FuzzyTokenMatch& match, const std::string& target, int offset) const;
		bool TryFuzzyMatch(FuzzyTokenMatch& match, const std::string& target, int offset, int missBudget) const;
	};

	class FuzzyTokenMatch
	{
		friend struct FuzzySearchToken;
		friend class FuzzySearchResult;
		friend class FuzzySearch;

		int MatchedLength = 0;
		int TokenLength = 0;
		int TokenIndex = -1;
		SVector2<I32> Interval = Havtorn::SVector2<I32>(-1, -1); // x and y are both inclusive indices.

		void AddSubString(I32 subStringStart, I32 subStringLength);
		bool Intersects(const Havtorn::SVector2<I32>& otherInterval) const;
		bool IsCaseInsensitive(const String& original, const String& adjusted) const;
		I32 GetMissCount() const { return TokenLength - MatchedLength; }

	public:
		I32 Score = 0;
		std::vector<Havtorn::SVector2<U32>> SubStringIndexPairs; // x is start index, y is length.
	};

	class FuzzySearchResult
	{
		friend class FuzzySearch;
	public:
		I32 Score = 0;
		I32 DirIndex = -1;
		String Target;
		std::vector<FuzzyTokenMatch> TokenMatches;

	private:
		int MissBudget = 1;
		Havtorn::SVector2<I32> MatchInterval = Havtorn::SVector2<I32>(-1, -1);

		bool CanAddTokenMatch(const FuzzyTokenMatch& match) const;
		void ScoreTokenMatch(FuzzyTokenMatch& match, bool caseInsensitive) const;
		void AddTokenMatch(const FuzzyTokenMatch& match);
		void MaybeApplyScoreBonus();
	};

	class FuzzySearch
	{
	public:
		int StartOffset = 0;
		int MaxMisses = 2;
		int MaxResults = 10;
		bool CaseSensitive = false;
		bool AllowSubsequences = true;

		void SetQuery(const String& query);
		bool Search(const String& target, FuzzySearchResult& result) const;
		void SearchAll(const std::vector<std::string>& targets, std::vector<FuzzySearchResult>& results) const;
		void SortAndFilter(std::vector<FuzzySearchResult>& results) const;

	private:
		std::vector<FuzzySearchToken> Tokens;
	};
}