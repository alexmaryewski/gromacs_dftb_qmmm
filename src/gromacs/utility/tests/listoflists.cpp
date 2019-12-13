/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 2018,2019, by the GROMACS development team, led by
 * Mark Abraham, David van der Spoel, Berk Hess, and Erik Lindahl,
 * and including many others, as listed in the AUTHORS file in the
 * top-level source directory and at http://www.gromacs.org.
 *
 * GROMACS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * GROMACS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GROMACS; if not, see
 * http://www.gnu.org/licenses, or write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *
 * If you want to redistribute modifications to GROMACS, please
 * consider that scientific software is very special. Version
 * control is crucial - bugs must be traceable. We will be happy to
 * consider code for inclusion in the official distribution, but
 * derived work must not be called official GROMACS. Details are found
 * in the README & COPYING files - if they are missing, get the
 * official version at http://www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the research papers on the package. Check out http://www.gromacs.org.
 */
/*! \internal \file
 * \brief
 * Tests for the ListOfLists class.
 *
 * \author berk Hess <hess@kth.se>
 * \ingroup module_utility
 */
#include "gmxpre.h"

#include "gromacs/utility/listoflists.h"

#include <gtest/gtest.h>

#include "testutils/testasserts.h"
#include "testutils/testmatchers.h"

namespace gmx
{

namespace
{

//! Compares all element between two lists of lists
template<typename T>
void compareLists(const ListOfLists<T>& list, const std::vector<std::vector<T>>& v)
{
    using ::testing::Eq;
    using ::testing::Pointwise;

    ASSERT_EQ(list.size(), v.size());
    for (std::size_t i = 0; i < list.size(); i++)
    {
        ASSERT_EQ(list[i].size(), v[i].size());
        EXPECT_THAT(list[i], Pointwise(Eq(), v[i]));
    }
}

TEST(ListOfLists, EmptyListOfListsWorks)
{
    ListOfLists<char> list;

    EXPECT_EQ(list.size(), 0);
    EXPECT_EQ(list.empty(), true);
    EXPECT_EQ(list.numElements(), 0);
}

//! Checks whether append works and stores the data correctly
template<typename T>
void checkAppend(const std::vector<std::vector<T>> inputLists)
{
    ListOfLists<T> list;

    for (const auto& inputList : inputLists)
    {
        list.pushBack(inputList);
    }
    EXPECT_EQ(list.size(), 2);
    compareLists(list, inputLists);
}

TEST(ListOfLists, AppendPodWorks)
{
    const std::vector<std::vector<char>> v = { { 5, 3 }, { char(-1), 7, 4 } };

    checkAppend(v);
}

TEST(ListOfLists, AppendNonpodWorks)
{
    const std::vector<std::vector<std::string>> v = { { "Will", "this" },
                                                      { "test", "work", "", "?" } };

    checkAppend(v);
}

TEST(ListOfLists, EmptyListWorks)
{
    ListOfLists<char> list;

    std::vector<char> v = { 5, 3 };
    list.pushBack(v);
    list.pushBack({});
    EXPECT_EQ(list.size(), 2);
    auto a = list[1];
    EXPECT_EQ(a.empty(), true);
}

TEST(ListOfLists, ClearWorks)
{
    ListOfLists<char> list;

    std::vector<char> v = { 5, 3 };
    list.pushBack(v);
    list.pushBack({});
    list.clear();
    EXPECT_EQ(list.empty(), true);
    EXPECT_EQ(list.numElements(), 0);
}

TEST(ListOfLists, OutOfRangeAccessThrows)
{
    ListOfLists<char> list;

    std::vector<char> v = { 5, 3 };
    EXPECT_THROW(list.at(1), std::out_of_range);
}

TEST(ListOfLists, ExtractsAndRestores)
{
    const std::vector<std::vector<char>> v({ { 5, 3 }, {}, { char(-1), 4 } });

    ListOfLists<char> list1;
    for (const auto& vlist : v)
    {
        list1.pushBack(vlist);
    }

    auto             listRanges = list1.listRangesView();
    auto             elements   = list1.elementsView();
    std::vector<int> listRangesVector;
    listRangesVector.insert(listRangesVector.begin(), listRanges.begin(), listRanges.end());
    std::vector<char> elementsVector;
    elementsVector.insert(elementsVector.begin(), elements.begin(), elements.end());
    ListOfLists<char> list2(std::move(listRangesVector), std::move(elementsVector));
    compareLists(list2, v);
}

TEST(ListOfLists, AppendsListOfListsWithOffset)
{
    std::vector<std::vector<char>> v = { { 5, 3 }, { 2, char(-1) }, { 4 } };

    ListOfLists<char> list1;
    ListOfLists<char> list2;

    list1.pushBack(v[0]);
    list2.pushBack(v[1]);
    list2.pushBack(v[2]);
    const char offset = 2;
    list1.appendListOfLists(list2, offset);
    for (std::size_t i = 1; i < v.size(); i++)
    {
        for (auto& elem : v[i])
        {
            elem += offset;
        }
    }
    compareLists(list1, v);
}

TEST(ListOfLists, AppendsListOfListsNonpod)
{
    std::vector<std::vector<std::string>> v({ { "a", "bc" }, { "d" }, {}, { "efg", "h" } });

    ListOfLists<std::string> list1;
    ListOfLists<std::string> list2;

    list1.pushBack(v[0]);
    list1.pushBack(v[1]);
    list2.pushBack(v[2]);
    list2.pushBack(v[3]);
    list1.appendListOfLists(list2);
    compareLists(list1, v);
}

} // namespace

} // namespace gmx
