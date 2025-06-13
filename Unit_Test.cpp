// =====================================================================================
//
//       Filename:  Unit_Test.cpp
//
//    Description:  Driver program for Unit tests
//
//        Version:  1.0
//        Created:  10/24/2018 9:45:53 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  David P. Riedel (dpr), driedel@cox.net
//        License:  GNU General Public License v3
//        Company:
//
// =====================================================================================

/* This file is part of Extractor_Markup. */

/* Extractor_Markup is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* Extractor_Markup is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with Extractor_Markup.  If not, see <http://www.gnu.org/licenses/>. */

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <system_error>
#include <thread>

// #include <boost/algorithm/string/predicate.hpp>

#include <gmock/gmock.h>

#include <range/v3/all.hpp>

#include <spdlog/spdlog.h>

#include "AnchorsFromHTML.h"
#include "Extractor_HTML_FileFilter.h"
#include "Extractor_Utils.h"
#include "HTML_FromFile.h"
#include "SEC_Header.h"
#include "SharesOutstanding.h"
#include "TablesFromFile.h"

namespace fs = std::filesystem;

namespace rng = ranges;

using namespace testing;

// some specific files for Testing.

const EM::FileName FILE_WITH_HTML_10Q{"/vol_DA/SEC/Archives/edgar/data/1046672/0001102624-13-001243.txt"};
const EM::FileName FILE_WITH_XML_10Q{"/vol_DA/SEC/Archives/edgar/data/1460602/0001062993-13-005017.txt"};
const EM::FileName FILE_WITH_XML2_10Q{"/vol_DA/SEC/Archives/edgar/data/1341319/0001445866-13-001158.txt"};
const EM::FileName FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS{
    "/vol_DA/SEC/Archives/edgar/data/1046672/0001102624-13-001243.txt"};
const EM::FileName FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS2{
    "/vol_DA/SEC/Archives/edgar/data/841360/0001086380-13-000031.txt"};
const EM::FileName FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS3{
    "/vol_DA/SEC/Archives/edgar/data/949268/0001137171-09-000933.txt"};
const EM::FileName FILE_WITH_HTML_10Q_WITH_ANCHORS{"/vol_DA/SEC/Archives/edgar/data/1420525/0001420525-09-000028.txt"};
const EM::FileName FILE_WITH_HTML_10Q_WITH_ANCHORS2{"/vol_DA/SEC/Archives/edgar/data/1434743/0001193125-09-235665.txt"};
const EM::FileName FILE_WITH_HTML_10Q_WITH_ANCHORS3{"/vol_DA/SEC/Archives/edgar/data/1326688/0001104659-09-064933.txt"};
const EM::FileName FILE_WITH_HTML_10Q_WITH_ANCHORS4{"/vol_DA/SEC/Archives/edgar/data/1466739/0001002014-13-000458.txt"};
const EM::FileName FILE_WITH_HTML_10Q_WITH_ANCHORS5{"/vol_DA/SEC/Archives/edgar/data/1460602/0001062993-13-005017.txt"};
// const EM::FileName FILE_WITH_HTML_10Q_WITH_ANCHORS{"/tmp/x1.html"};
const EM::FileName FILE_WITH_XML_10K{"/vol_DA/SEC/Archives/edgar/data/google-10k.txt"};
const EM::FileName FILE_WITH_HTML_10Q_MINIMAL_DATA{"/vol_DA/SEC/Archives/edgar/data/841360/0001086380-13-000030.txt"};
const EM::FileName FILE_WITH_HTML_10Q_PROBLEM_REGEX1{
    "/vol_DA/SEC/Archives/edgar/data/1377936/0001104659-13-075719.txt"};
const EM::FileName FILE_WITH_HTML_10Q_PROBLEM_REGEX2{"/vol_DA/SEC/Archives/edgar/data/4515/0000004515-13-000053.txt"};
const EM::FileName FILE_WITH_HTML_10Q_PROBLEM_WITH_ASSETS1{
    "/vol_DA/SEC/Archives/edgar/data/68270/0000068270-13-000059.txt"};
const EM::FileName FILE_WITH_NO_HTML_10Q{"/vol_DA/SEC/SEC_forms/0000855931/10-Q/0001130319-01-500242.txt"};
const EM::FileName FILE_WITH_BAD_CASH_FLOW_VALUE{"/vol_DA/SEC/Archives/edgar/data/949982/0001477932-13-004674.txt"};

const EM::FileName FILE_WITH_NO_HTML_AND_NO_FILENAME_10K{
    "/vol_DA/SEC/SEC_forms/0001000015/10-K/0000912057-00-014793.txt"};

const EM::FileName FILE_WITH_NO_HTML2_10Q{"/vol_DA/SEC/Archives/edgar/data/1421907/0001165527-13-000854.txt"};
const EM::FileName FILE_WITH_ANCHOR_LOOP{"/vol_DA/SEC/SEC_forms/0000758938/10-K/0000950124-06-005605.txt"};
const EM::FileName SEC_DIRECTORY{"/vol_DA/SEC/Archives/edgar/data"};
// const EM::FileName
// FILE_NO_NAMESPACE_10Q{"/vol_DA/SEC/Archives/edgar/data/68270/0000068270-13-000059.txt"};
// const EM::FileName
// FILE_SOME_NAMESPACE_10Q{"/vol_DA/SEC/Archives/edgar/data/1552979/0001214782-13-000386.txt"};
// const EM::FileName
// FILE_MULTIPLE_LABEL_LINKS{"/vol_DA/SEC/Archives/edgar/data/1540334/0001078782-13-002015.txt"};
// const EM::FileName
// BAD_FILE1{"/vol_DA/SEC/SEC_forms/1000228/10-K/0001000228-11-000014.txt"};
// const EM::FileName
// BAD_FILE2{"/vol_DA/SEC/SEC_forms/1000180/10-K/0001000180-16-000068.txt"};
// const EM::FileName
// BAD_FILE3{"/vol_DA/SEC/SEC_forms/1000697/10-K/0000950123-11-018381.txt"};
// const EM::FileName
// NO_SHARES_OUT{"/vol_DA/SEC/SEC_forms/1023453/10-K/0001144204-12-017368.txt"};
// const EM::FileName TEST_FILE_LIST{"./list_with_bad_file.txt"};
// const EM::FileName
// MISSING_VALUES1_10K{"/vol_DA/SEC/SEC_forms/1004980/10-K/0001193125-12-065537.txt"};
// const EM::FileName
// MISSING_VALUES2_10K{"/vol_DA/SEC/SEC_forms/1002638/10-K/0001193125-09-179839.txt"};

const EM::FileName FILE_SHOWING_DUPLICATE_LABEL_TEXT{
    "/vol_DA/SEC/Archives/edgar/data/1522222/0001185185-13-002216.txt"};
const EM::FileName FILE_WITH_HTML_10Q_FIND_SHARES1{"/vol_DA/SEC/Archives/edgar/data/29989/0000029989-13-000015.txt"};
const EM::FileName FILE_WITH_HTML_NO_HREFS1_10K{"/vol_DA/SEC/SEC_forms/0000906345/10-K/0000906345-04-000036.txt"};
const EM::FileName FILE_WITH_HTML_ANCHORS_10Q{"/home/dpriedel/projects/Extractor_Markup/YUM_bad_balsheet.html"};
const EM::FileName FILE_WITH_HTML_10Q_WITH_SEGMENTED_ANCHORS{
    "/home/dpriedel/projects/Extractor_HTML_Test/test_files/RubyTuesday.html"};

const EM::FileName TRAINING_FILE_FOR_SHARES_OUTSTANDING{"/home/dpriedel/projects/Extractor_HTML_Test/"
                                                        "shares_outstanding_training_file.txt"};

const EM::FileName FILE_WITH_SHARES_AND_VALUE{"/vol_DA/SEC/Archives/edgar/html/0000004127/"
                                              "0000004127-15-000037.txt_fy1510k1022015.htm"};
const EM::FileName FILE_WITH_NO_POSSIBLES{"/vol_DA/SEC/Archives/edgar/html/0001609711/"
                                          "0001609711-19-000012.txt_a201810-kx10kdocument.htm"};
const EM::FileName FILE_WITH_SHARES_BEFORE_YESNO{"/vol_KUtil2/SEC_exports/html/0000019617/10-K/"
                                                 "0000950123-04-002022.txt_y94051e10vk.htm"};
const EM::FileName FILE_WITH_SHARES_AFTER_CONTENTS{"/vol_KUtil2/SEC_exports/html/0000000020/10-K/"
                                                   "0000893220-05-000728.txt_w06061e10vk.htm"};
const EM::FileName FILE_WITH_SHARE_VALUE_NO_DOLLAR_SIGN{"/vol_KUtil2/SEC_exports/html/0001326160/10-K/"
                                                        "0001326160-14-000003.txt_form10k.htm"};
const EM::FileName FILE_WITH_XBRL_INSIDE_HTM_DOCUMENT{"/vol_KUtil2/SEC_exports/html/0000029905/10-K/"
                                                      "0000029905-19-000019.txt_wfx-20181231.htm"};
const EM::FileName FILE_WITH_XBRL_INSIDE_TXT_DOCUMENT{"/vol_DA/SEC/SEC_forms/0000029905/10-K/0000029905-19-000019.txt"};

// This ctype facet does NOT classify spaces and tabs as whitespace
// from cppreference example

struct line_only_whitespace : std::ctype<char>
{
    static const mask *make_table()
    {
        // make a copy of the "C" locale table
        static std::vector<mask> v(classic_table(), classic_table() + table_size);
        v['\t'] &= ~space; // tab will not be classified as whitespace
        v[' '] &= ~space;  // space will not be classified as whitespace
        return &v[0];
    }
    explicit line_only_whitespace(std::size_t refs = 0) : ctype(make_table(), false, refs)
    {
    }
};

// some utility functions for testing.

class Iterators : public Test
{
};

TEST_F(Iterators, HTMLIteratorFileWithHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile html{&sections, FILE_WITH_HTML_10Q};

    auto how_many = std::distance(std::begin(html), std::end(html));
    EXPECT_EQ(how_many, 5);

    auto htmls = Find_HTML_Documents(&sections, FILE_WITH_HTML_10Q);
    ASSERT_EQ(htmls.size(), 5);
}

TEST_F(Iterators, HTMLRangeFileWithHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile html{&sections, FILE_WITH_HTML_10Q};

    auto how_many = rng::distance(html);
    EXPECT_TRUE(how_many == 5);

    auto htmls = Find_HTML_Documents(&sections, FILE_WITH_HTML_10Q);
    ASSERT_TRUE(htmls.size() == 5);
}

TEST_F(Iterators, HTMLIteratorFileWithMinimalHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_NO_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile html{&sections, FILE_WITH_NO_HTML_10Q};

    auto how_many = std::distance(std::begin(html), std::end(html));

    EXPECT_TRUE(how_many == 0);

    auto htmls = Find_HTML_Documents(&sections, FILE_WITH_NO_HTML_10Q);
    ASSERT_TRUE(htmls.size() == 0);
}

TEST_F(Iterators, HTMLRangeFileWithMinimalHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_NO_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile html{&sections, FILE_WITH_NO_HTML_10Q};

    auto how_many = rng::distance(html);

    EXPECT_TRUE(how_many == 0);

    auto htmls = Find_HTML_Documents(&sections, FILE_WITH_NO_HTML_10Q);
    ASSERT_TRUE(htmls.size() == 0);
}

TEST_F(Iterators, HTMLIteratorFileWithHTML_10K)
{
    const auto file_content_10K = LoadDataFileForUse(FILE_WITH_XML_10K);
    EM::FileContent file_content{file_content_10K};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile html{&sections, FILE_WITH_XML_10K};

    auto how_many = std::distance(std::begin(html), std::end(html));
    EXPECT_TRUE(how_many == 107);

    auto htmls = Find_HTML_Documents(&sections, FILE_WITH_XML_10K);
    ASSERT_TRUE(htmls.size() == 107);
}

TEST_F(Iterators, AnchorIteratorFileWithHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile htmls{&sections, FILE_WITH_HTML_10Q};

    AnchorsFromHTML anchors(htmls.begin()->html_);

    auto how_many = std::distance(std::begin(anchors), std::end(anchors));
    EXPECT_EQ(how_many, 22);

    // add a cache test

    how_many = std::distance(std::begin(anchors), std::end(anchors));
    ASSERT_EQ(how_many, 22);
}

TEST_F(Iterators, AnchorRangeFileWithHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile htmls{&sections, FILE_WITH_HTML_10Q};

    AnchorsFromHTML anchors{rng::front(htmls).html_};

    auto how_many = rng::distance(anchors);
    EXPECT_EQ(how_many, 22);

    // add a cache test

    how_many = rng::distance(anchors);
    ASSERT_EQ(how_many, 22);
}

TEST_F(Iterators, AnchorIteratorFileWithXML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_XML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile htmls{&sections, FILE_WITH_XML_10Q};
    int total = 0;
    for (const auto &html : htmls)
    {
        AnchorsFromHTML anchors(html.html_);
        total += std::distance(std::begin(anchors), std::end(anchors));
    }
    std::cout << "Total anchors found: " << total << '\n';
    ASSERT_TRUE(total == 2239);
}

TEST_F(Iterators, AnchorRangeFileWithXML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_XML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile htmls{&sections, FILE_WITH_XML_10Q};

    int total = rng::accumulate(htmls | rng::views::transform([](const auto &html) {
                                    AnchorsFromHTML x{html.html_};
                                    return rng::distance(x);
                                }),
                                0);
    std::cout << "Total anchors found: " << total << '\n';
    ASSERT_TRUE(total == 2239);
}

TEST_F(Iterators, TableIteratorFileWithHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile htmls{&sections, FILE_WITH_HTML_10Q};

    TablesFromHTML tables(htmls.begin()->html_);

    auto how_many = std::distance(std::begin(tables), std::end(tables));

    // a bunch of tables are skipped because they have too little html
    // so there are 49 usable tables left.

    EXPECT_EQ(how_many, 49);

    // add a cache test

    how_many = std::distance(std::begin(tables), std::end(tables));
    ASSERT_EQ(how_many, 49);
}

TEST_F(Iterators, TableRangeIteratorFileWithHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile htmls{&sections, FILE_WITH_HTML_10Q};

    TablesFromHTML tables(htmls.begin()->html_);

    auto how_many = rng::distance(tables);

    // a bunch of tables are skipped because they have too little html
    // so there are 49 usable tables left.

    EXPECT_EQ(how_many, 49);

    // add a cache test

    how_many = rng::distance(tables);
    ASSERT_EQ(how_many, 49);
}

class IdentifyHTMLFilesToUse : public Test
{
};

TEST_F(IdentifyHTMLFilesToUse, ConfirmFileHasHTML)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    SEC_Header SEC_data;
    SEC_data.UseData(file_content);
    SEC_data.ExtractHeaderFields();

    FileHasHTML filter1{{"10-Q"}};
    auto use_file = filter1(SEC_data.GetFields(), sections);
    ASSERT_TRUE(use_file);
}

TEST_F(IdentifyHTMLFilesToUse, DISABLED_ConfirmFileHasXML)
{
    // disabled because we now process input files with XML.
    //
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_XML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    FileHasHTML filter1{{"10-Q"}};
    auto use_file = filter1(EM::SEC_Header_fields{}, sections);
    ASSERT_FALSE(use_file);
}

class LocateDocumentWithFinancialContent : public Test
{
};

TEST_F(LocateDocumentWithFinancialContent, FileHasHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile htmls{&sections, FILE_WITH_HTML_10Q};

    FinancialDocumentFilter document_filter{{"10-Q"}};
    auto document = std::find_if(std::begin(htmls), std::end(htmls), document_filter);
    ASSERT_TRUE(document != htmls.end());
}

TEST_F(LocateDocumentWithFinancialContent, FileHasXML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_XML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile htmls{&sections, FILE_WITH_XML_10Q};

    FinancialDocumentFilter document_filter{{"10-Q"}};
    auto document = rng::find_if(htmls, document_filter);
    ASSERT_TRUE(document != htmls.end());
}

TEST_F(LocateDocumentWithFinancialContent, FileHasNoUsableAnchors_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile htmls{&sections, FILE_WITH_HTML_10Q_WITH_ANCHORS};

    FinancialDocumentFilter document_filter{{"10-Q"}};
    auto document = std::find_if(std::begin(htmls), std::end(htmls), document_filter);
    ASSERT_TRUE(document != htmls.end());
}

TEST_F(LocateDocumentWithFinancialContent, FindContentInFileHasMinimalData_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_MINIMAL_DATA);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile htmls{&sections, FILE_WITH_HTML_10Q_MINIMAL_DATA};

    FinancialDocumentFilter document_filter{{"10-Q"}};
    auto document = std::find_if(std::begin(htmls), std::end(htmls), document_filter);
    ASSERT_TRUE(document != htmls.end());
}

TEST_F(LocateDocumentWithFinancialContent, FindNoHTMLInFileWithNoHTML_10Q)
{
    // this should find nothing because our iterator will be 'empty'

    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_NO_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile htmls{&sections, FILE_WITH_NO_HTML_10Q};

    FinancialDocumentFilter document_filter{{"10-Q"}};
    auto document = std::find_if(std::begin(htmls), std::end(htmls), document_filter);
    ASSERT_TRUE(document == htmls.end());
}

class FindAnchorsForFinancialStatements : public Test
{
public:
};

TEST_F(FindAnchorsForFinancialStatements, FindSegmentedTopLevelAnchor_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_SEGMENTED_ANCHORS);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile htmls{&sections, FILE_WITH_HTML_10Q_WITH_SEGMENTED_ANCHORS};

    // we know there is only 1 HTML document in this file.

    auto financial_content = htmls.begin()->html_;

    AnchorsFromHTML anchors{financial_content};

    rng::for_each(anchors, [](const auto &anchor) {
        std::cout << "HREF: " << anchor.href_ << "\n\tNAME: " << anchor.name_ << "\n\tTEXT: " << anchor.text_
                  << "\n\tCONTENT: " << anchor.anchor_content_.get() << '\n';
    });

    auto found_some = rng::adjacent_find(anchors, [](const auto &a, const auto &b) { return a.href_ == b.href_; });
    EXPECT_TRUE(found_some != anchors.end());
}

TEST_F(FindAnchorsForFinancialStatements, FindTopLevelAnchor_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    auto financial_content = FindFinancialContentUsingAnchors(&sections, FILE_WITH_HTML_10Q_WITH_ANCHORS);

    ASSERT_TRUE(financial_content);
}

TEST_F(FindAnchorsForFinancialStatements, FindTopLevelAnchorInFileWithMinimalHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_MINIMAL_DATA);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    auto financial_content = FindFinancialContentUsingAnchors(&sections, FILE_WITH_HTML_10Q_MINIMAL_DATA);

    ASSERT_TRUE(financial_content);
}

TEST_F(FindAnchorsForFinancialStatements, DontFindTopLevelAnchorInFileWithNoHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_NO_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    auto financial_content = FindFinancialContentUsingAnchors(&sections, FILE_WITH_NO_HTML_10Q);

    ASSERT_FALSE(financial_content);
}

// TEST_F(FindAnchorsForFinancialStatements, FindAnchors_10Q)
//{
//     auto file_content_10Q =
//     LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS);
//
//     AnchorList statement_anchors;
//
//     auto financial_content =
//     FindFinancialContentUsingAnchors(file_content_10Q); if (!
//     financial_content.empty())
//     {
//         AnchorsFromHTML anchors(financial_content);
//         std::copy_if(anchors.begin(),
//                 anchors.end(),
//                 std::back_inserter(statement_anchors),
//                 FinancialAnchorFilter
//                 );
//     }
//     std::cout << "statement anchors: " << statement_anchors.size() << '\n';
//     ASSERT_TRUE(statement_anchors.size() == 4);
// }
//
// TEST_F(FindAnchorsForFinancialStatements, FindAnchorsMinimalHTML_10Q)
//{
//     auto file_content_10Q =
//     LoadDataFileForUse(FILE_WITH_HTML_10Q_MINIMAL_DATA);
//
//     AnchorList statement_anchors;
//
//     auto financial_content =
//     FindFinancialContentUsingAnchors(file_content_10Q); if (!
//     financial_content.empty())
//     {
//         AnchorsFromHTML anchors(financial_content);
//         std::copy_if(anchors.begin(),
//                 anchors.end(),
//                 std::back_inserter(statement_anchors),
//                 FinancialAnchorFilter
//                 );
//     }
//     std::cout << "statement anchors: " << statement_anchors.size() << '\n';
//     ASSERT_TRUE(statement_anchors.size() == 3);
// }

TEST_F(FindAnchorsForFinancialStatements, FindAnchorsComplexHTML_NO_HREFS_10K)
{
    const auto file_content_10K = LoadDataFileForUse(FILE_WITH_HTML_NO_HREFS1_10K);
    EM::FileContent file_content{file_content_10K};
    const auto sections = LocateDocumentSections(file_content);

    AnchorList statement_anchors;

    auto financial_content = FindFinancialContentUsingAnchors(&sections, FILE_WITH_HTML_NO_HREFS1_10K);
    ASSERT_FALSE(financial_content);
}

TEST_F(FindAnchorsForFinancialStatements, FindAnchorDestinations_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    auto financial_document = FindFinancialContentUsingAnchors(&sections, FILE_WITH_HTML_10Q_WITH_ANCHORS);

    AnchorsFromHTML anchors(financial_document->first);

    static const boost::regex regex_balance_sheet{R"***((?:balance\s+sheet)|(?:financial.*?position))***",
                                                  boost::regex_constants::normal | boost::regex_constants::icase};
    auto balance_sheet_href =
        rng::find_if(anchors, [](const auto &anchor) { return AnchorFilterUsingRegex(regex_balance_sheet, anchor); });

    static const boost::regex regex_operations{R"***((?:statement|statements)\s+?of.*?(?:oper|loss|income|earning))***",
                                               boost::regex_constants::normal | boost::regex_constants::icase};

    auto stmt_of_ops_href =
        rng::find_if(anchors, [](const auto &anchor) { return AnchorFilterUsingRegex(regex_operations, anchor); });

    static const boost::regex regex_cash_flow{R"***((?:cash\s+flow)|(?:statement.+?cash)|(?:cashflow))***",
                                              boost::regex_constants::normal | boost::regex_constants::icase};

    auto cash_flows_href =
        rng::find_if(anchors, [](const auto &anchor) { return AnchorFilterUsingRegex(regex_cash_flow, anchor); });

    //    auto sholder_equity_href = std::find_if(anchors.begin(), anchors.end(),
    //    StockholdersEquityAnchorFilter);

    EXPECT_TRUE(balance_sheet_href != anchors.end());
    EXPECT_TRUE(stmt_of_ops_href != anchors.end());
    EXPECT_TRUE(cash_flows_href != anchors.end());
    //    EXPECT_TRUE(sholder_equity_href != anchors.end());

    auto balance_sheet_dest = FindDestinationAnchor(*balance_sheet_href, anchors);
    auto stmt_of_ops_dest = FindDestinationAnchor(*stmt_of_ops_href, anchors);
    auto cash_flows_dest = FindDestinationAnchor(*cash_flows_href, anchors);
    //    auto sholder_equity_dest = FindDestinationAnchor(*sholder_equity_href,
    //    anchors);

    EXPECT_TRUE(balance_sheet_dest != anchors.end());
    EXPECT_TRUE(stmt_of_ops_dest != anchors.end());
    EXPECT_TRUE(cash_flows_dest != anchors.end());
    //    EXPECT_TRUE(sholder_equity_dest != anchors.end());
}

// TEST_F(FindAnchorsForFinancialStatements,
// FindAnchorDestinationsMinimalHTML_10Q)
//{
//     auto file_content_10Q =
//     LoadDataFileForUse(FILE_WITH_HTML_10Q_MINIMAL_DATA); documents =
//     LocateDocumentSections(file_content_10Q);
//
//     auto all_anchors = FindAllDocumentAnchors(documents);
//     auto statement_anchors = FilterFinancialAnchors(all_anchors);
//     auto destination_anchors = FindAnchorDestinations(statement_anchors,
//     all_anchors); std::cout << "\nDestination Anchors: \n"; for (const auto&
//     anchor : destination_anchors)
//     {
//         std::cout
//             << "HREF: " << anchor.href
//             << "\tNAME: " << anchor.name
//             << "\tTEXT: " << anchor.text
//             << "\tCONTENT: " << anchor.anchor_content << '\n';
//     }
//
//     ASSERT_TRUE(destination_anchors.size() == 3);
// }
//
class FindIndividualFinancialStatements_10Q : public Test
{
public:
};

TEST_F(FindIndividualFinancialStatements_10Q, FindBalanceSheetInFileWithHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile htmls{&sections, FILE_WITH_HTML_10Q};

    bool found_it = false;

    FinancialDocumentFilter document_filter{{"10-Q"}};
    for (auto html : htmls)
    {
        if (document_filter(html))
        {
            TablesFromHTML tables{html.html_};
            auto balance_sheet = std::find_if(tables.begin(), tables.end(), [](const auto &x) {
                return BalanceSheetFilter(x.current_table_parsed_);
            });
            if (balance_sheet != tables.end())
            {
                found_it = true;
                break;
            }
        }
    }

    ASSERT_TRUE(found_it);
}

TEST_F(FindIndividualFinancialStatements_10Q, RangeFindBalanceSheetInFileWithHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile htmls{&sections, FILE_WITH_HTML_10Q};

    FinancialDocumentFilter document_filter{{"10-Q"}};

    auto find_doc = rng::views::filter(document_filter) | rng::views::filter([](const auto &html) {
                        TablesFromHTML ts{html.html_};
                        return !(ts | rng::views::filter([](const auto &t) {
                                     return BalanceSheetFilter(t.current_table_parsed_);
                                 })).empty();
                    });

    bool found_it = !(htmls | find_doc).empty();

    ASSERT_TRUE(found_it);
}

TEST_F(FindIndividualFinancialStatements_10Q, FindBalanceSheetWithAnchorsHTML5_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS5);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    auto financial_content = FindFinancialContentUsingAnchors(&sections, FILE_WITH_HTML_10Q_WITH_ANCHORS5);
    EXPECT_TRUE(financial_content);

    AnchorsFromHTML anchors(financial_content->first);

    static const boost::regex regex_balance_sheet{R"***((?:balance\s+sheet)|(?:financial.*?position))***",
                                                  boost::regex_constants::normal | boost::regex_constants::icase};

    auto balance_sheet =
        FindStatementContent<BalanceSheet>(financial_content->first, anchors, regex_balance_sheet, BalanceSheetFilter);
    ASSERT_TRUE(!balance_sheet.empty());
}

TEST_F(FindIndividualFinancialStatements_10Q, FindStatementOfOperationsWithAnchorsHTML5_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS5);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    auto financial_content = FindFinancialContentUsingAnchors(&sections, FILE_WITH_HTML_10Q_WITH_ANCHORS5);
    EXPECT_TRUE(financial_content);

    static const boost::regex regex_operations{R"***((?:statement|statements)\s+?of.*?(?:oper|loss|income|earning))***",
                                               boost::regex_constants::normal | boost::regex_constants::icase};

    AnchorsFromHTML anchors(financial_content->first);

    auto statement_of_operations = FindStatementContent<StatementOfOperations>(
        financial_content->first, anchors, regex_operations, StatementOfOperationsFilter);
    ASSERT_TRUE(!statement_of_operations.empty());
}

TEST_F(FindIndividualFinancialStatements_10Q, FindCashFlowsWithAnchorsHTML5_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS5);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    auto financial_content = FindFinancialContentUsingAnchors(&sections, FILE_WITH_HTML_10Q_WITH_ANCHORS5);
    EXPECT_TRUE(financial_content);

    static const boost::regex regex_cash_flow{R"***((?:cash\s+flow)|(?:statement.+?cash)|(?:cashflow))***",
                                              boost::regex_constants::normal | boost::regex_constants::icase};

    AnchorsFromHTML anchors(financial_content->first);

    auto cash_flows =
        FindStatementContent<CashFlows>(financial_content->first, anchors, regex_cash_flow, CashFlowsFilter);

    ASSERT_TRUE(!cash_flows.empty());
}

TEST_F(FindIndividualFinancialStatements_10Q, FindStatementOfOperations_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    HTML_FromFile htmls{&sections, FILE_WITH_HTML_10Q};

    bool found_it = false;

    FinancialDocumentFilter document_filter{{"10-Q"}};
    for (auto html : htmls)
    {
        if (document_filter(html))
        {
            TablesFromHTML tables{html.html_};
            auto statement_of_ops = std::find_if(tables.begin(), tables.end(), [](const auto &x) {
                return StatementOfOperationsFilter(x.current_table_parsed_);
            });
            if (statement_of_ops != tables.end())
            {
                found_it = true;
                break;
            }
        }
    }

    ASSERT_TRUE(found_it);
}

TEST_F(FindIndividualFinancialStatements_10Q, FindCashFlowStatement_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    HTML_FromFile htmls{&sections, FILE_WITH_HTML_10Q};

    bool found_it = false;

    FinancialDocumentFilter document_filter{{"10-Q"}};
    for (auto html : htmls)
    {
        if (document_filter(html))
        {
            TablesFromHTML tables{html.html_};
            auto cash_flows = std::find_if(tables.begin(), tables.end(),
                                           [](const auto &x) { return CashFlowsFilter(x.current_table_parsed_); });
            if (cash_flows != tables.end())
            {
                found_it = true;
                break;
            }
        }
    }

    ASSERT_TRUE(found_it);
}

TEST_F(FindIndividualFinancialStatements_10Q, FindCashFlowStatement2_10Q)
{
    //    // use a different file here

    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS2);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    auto financial_content = FindFinancialContentUsingAnchors(&sections, FILE_WITH_HTML_10Q_WITH_ANCHORS2);
    EXPECT_TRUE(financial_content);

    AnchorsFromHTML anchors(financial_content->first);

    static const boost::regex regex_balance_sheet{R"***((?:balance\s+sheet)|(?:financial.*?position))***",
                                                  boost::regex_constants::normal | boost::regex_constants::icase};

    auto balance_sheet =
        FindStatementContent<BalanceSheet>(financial_content->first, anchors, regex_balance_sheet, BalanceSheetFilter);
    ASSERT_TRUE(!balance_sheet.empty());
}

TEST_F(FindIndividualFinancialStatements_10Q, DISABLED_FindStockholderEquity_10Q)
{
    //    // TODO: if can't find separate table, look into Balance sheet
    //
    //    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    //    auto document = FindFinancialDocument(file_content_10Q);
    //
    //    TablesFromHTML tables{document};
    //    auto stockholder_equity = std::find_if(tables.begin(), tables.end(),
    //    StockholdersEquityFilter);
    //
    //    ASSERT_TRUE(stockholder_equity == tables.end());
}

class Multipliers : public Test
{
public:
};

TEST_F(Multipliers, DISABLED_FindDollarMultipliers_10Q)
{
    // disabled because the anchors in this file are split into
    // mulitple pieces in the middle of phrases we are looking for.
    // TODO: ?? handle this ??

    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_ANCHORS_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    HTML_FromFile htmls{&sections, FILE_WITH_HTML_ANCHORS_10Q};

    for (auto html : htmls)
    {
        AnchorsFromHTML anchors(html.html_);
        for (const auto &anchor : anchors)
        {
            std::cout << "\nHREF: " << anchor.href_ << "\n\tName: " << anchor.name_
                      << "\n\tContent: " << anchor.anchor_content_.get() << '\n';
        }
    }
    auto financial_content = FindFinancialContentUsingAnchors(&sections, FILE_WITH_HTML_ANCHORS_10Q);

    ASSERT_TRUE(financial_content);
}

TEST_F(Multipliers, FindDollarMultipliers_10Q)
{
    // disabled because the anchors in this file are split into
    // mulitple pieces in the middle of phrases we are looking for.
    // TODO: ?? handle this ??

    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS2);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    auto financial_content = FindFinancialContentUsingAnchors(&sections, FILE_WITH_HTML_10Q_WITH_ANCHORS2);
    EXPECT_TRUE(financial_content);

    auto financial_statements = ExtractFinancialStatements(financial_content->first);
    ASSERT_TRUE(financial_statements.has_data());

    std::cout << "multiplier: " << financial_statements.balance_sheet_.multiplier_s_ << '\n';
}

// TEST_F(FindIndividualFinancialStatements_10Q, FindTables_10Q)
//{
//     TablesFromHTML tables{financial_content};
//
//     auto how_many = std::distance(std::begin(tables), std::end(tables));
//
//     ASSERT_TRUE(how_many == 86);
// }
//
class ProcessEntireFile_10Q : public Test
{
public:
};

TEST_F(ProcessEntireFile_10Q, ExtractAllNeededSections)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    const SharesOutstanding so;
    auto all_sections = FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_HTML_10Q_WITH_ANCHORS);

    ASSERT_TRUE(all_sections.has_data());
}

TEST_F(ProcessEntireFile_10Q, ExtractAllNeededSections2)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS2);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    const SharesOutstanding so;
    auto all_sections = FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_HTML_10Q_WITH_ANCHORS2);
    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);
    std::cout << "\n\nCash Flow\n";
    std::cout.write(all_sections.cash_flows_.parsed_data_.data(), 500);
    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(all_sections.statement_of_operations_.parsed_data_.data(), 500);
    std::cout << "\n\nShareholder Equity\n";
    std::cout.write(all_sections.stockholders_equity_.parsed_data_.data(),
                    std::min(500UL, all_sections.stockholders_equity_.parsed_data_.size()));

    ASSERT_TRUE(all_sections.has_data());
    //    for (const auto& [key, value] : all_sections.ListValues())
    //    {
    //        std::cout << "\nkey: " << key << " value: " << value << '\n';
    //    }
}

TEST_F(ProcessEntireFile_10Q, ExtractAllNeededSections3)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS3);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    FinancialDocumentFilter document_filter{{"10-Q"}};

    HTML_FromFile htmls{&sections, FILE_WITH_HTML_10Q_WITH_ANCHORS3};

    auto financial_content = std::find_if(std::begin(htmls), std::end(htmls), document_filter);
    EXPECT_TRUE(financial_content != htmls.end());

    auto all_sections = ExtractFinancialStatementsUsingAnchors(financial_content->html_);
    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);
    std::cout << "\n\nCash Flow\n";
    std::cout.write(all_sections.cash_flows_.parsed_data_.data(), 500);
    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(all_sections.statement_of_operations_.parsed_data_.data(), 500);
    //    std::cout << "\n\nShareholder Equity\n";
    //    std::cout.write(all_sections.stockholders_equity_.the_data_.data(),
    //    std::min(500UL, all_sections.stockholders_equity_.the_data_.size()));

    ASSERT_TRUE(all_sections.has_data());
}

TEST_F(ProcessEntireFile_10Q, ExtractAllNeededSections4)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS4);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    auto financial_content = FindFinancialContentUsingAnchors(&sections, FILE_WITH_HTML_10Q_WITH_ANCHORS4);
    EXPECT_TRUE(financial_content);

    auto all_sections = ExtractFinancialStatementsUsingAnchors(financial_content->first);
    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);
    std::cout << "\n\nCash Flow\n";
    std::cout.write(all_sections.cash_flows_.parsed_data_.data(), 500);
    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(all_sections.statement_of_operations_.parsed_data_.data(), 500);
    //    std::cout << "\n\nShareholder Equity\n";
    //    std::cout.write(all_sections.stockholders_equity_.the_data_.data(),
    //    std::min(500UL, all_sections.stockholders_equity_.the_data_.size()));

    ASSERT_TRUE(all_sections.has_data());
}

TEST_F(ProcessEntireFile_10Q, ExtractAllNeededSectionsMinimalHTMLData)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_MINIMAL_DATA);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    //    FinancialDocumentFilter document_filter{{"10-Q"}};
    //
    //    HTML_FromFile htmls{file_content_10Q};
    //
    //    auto financial_content = std::find_if(std::begin(htmls),
    //    std::end(htmls), document_filter); EXPECT_TRUE(financial_content !=
    //    htmls.end());
    //
    //    auto all_sections =
    //    ExtractFinancialStatementsUsingAnchors(financial_content->html_);
    const SharesOutstanding so;
    auto all_sections = FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_HTML_10Q_MINIMAL_DATA);

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);
    std::cout << "\n\nCash Flow\n";
    std::cout.write(all_sections.cash_flows_.parsed_data_.data(), 500);
    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(all_sections.statement_of_operations_.parsed_data_.data(), 500);
    std::cout << "\n\nShareholder Equity\n";
    std::cout.write(all_sections.stockholders_equity_.parsed_data_.data(),
                    std::min(500UL, all_sections.stockholders_equity_.parsed_data_.size()));
    ASSERT_TRUE(all_sections.has_data());
}

class ProblemFiles_10Q : public Test
{
};

TEST_F(ProblemFiles_10Q, FindSectionAnchors_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    auto financial_content = FindFinancialContentUsingAnchors(&sections, FILE_WITH_HTML_10Q_WITH_ANCHORS);
    EXPECT_TRUE(financial_content);

    AnchorsFromHTML anchors(financial_content->first);

    static const boost::regex regex_balance_sheet{R"***((?:balance\s+sheet)|(?:financial.*?position))***",
                                                  boost::regex_constants::normal | boost::regex_constants::icase};
    auto balance_sheet_href =
        rng::find_if(anchors, [](const auto &anchor) { return AnchorFilterUsingRegex(regex_balance_sheet, anchor); });

    static const boost::regex regex_operations{R"***((?:statement|statements)\s+?of.*?(?:oper|loss|income|earning))***",
                                               boost::regex_constants::normal | boost::regex_constants::icase};

    auto stmt_of_ops_href =
        rng::find_if(anchors, [](const auto &anchor) { return AnchorFilterUsingRegex(regex_operations, anchor); });

    static const boost::regex regex_cash_flow{R"***((?:cash\s+flow)|(?:statement.+?cash)|(?:cashflow))***",
                                              boost::regex_constants::normal | boost::regex_constants::icase};

    auto cash_flows_href =
        rng::find_if(anchors, [](const auto &anchor) { return AnchorFilterUsingRegex(regex_cash_flow, anchor); });

    //    auto sholder_equity_href = std::find_if(anchors.begin(), anchors.end(),
    //    StockholdersEquityAnchorFilter);

    EXPECT_TRUE(balance_sheet_href != anchors.end());
    EXPECT_TRUE(stmt_of_ops_href != anchors.end());
    EXPECT_TRUE(cash_flows_href != anchors.end());

    auto balance_sheet_dest = FindDestinationAnchor(*balance_sheet_href, anchors);
    auto stmt_of_ops_dest = FindDestinationAnchor(*stmt_of_ops_href, anchors);
    auto cash_flows_dest = FindDestinationAnchor(*cash_flows_href, anchors);

    EXPECT_TRUE(balance_sheet_dest != anchors.end());
    EXPECT_TRUE(stmt_of_ops_dest != anchors.end());
    EXPECT_TRUE(cash_flows_dest != anchors.end());
}

TEST_F(ProblemFiles_10Q, FileWithMinimalData)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_MINIMAL_DATA);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    const SharesOutstanding so;
    auto financial_statements =
        FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_HTML_10Q_MINIMAL_DATA);

    ASSERT_TRUE(financial_statements.has_data());
}

class ProblemFiles_10K : public Test
{
};

TEST_F(ProblemFiles_10K, FileWithMinimalDataAndNoFileNme)
{
    const auto file_content_10K = LoadDataFileForUse(FILE_WITH_NO_HTML_AND_NO_FILENAME_10K);
    EM::FileContent file_content{file_content_10K};
    const auto sections = LocateDocumentSections(file_content);
    const SharesOutstanding so;

    ASSERT_THROW(FindAndExtractFinancialStatements(so, &sections, {"10-K"}, FILE_WITH_NO_HTML_AND_NO_FILENAME_10K),
                 ExtractorException);
}

TEST_F(ProblemFiles_10K, DISABLED_FindSectionAnchors_10K)
{
    // disabled because file does not have usable financial anchors.

    const auto file_content_10K = LoadDataFileForUse(FILE_WITH_ANCHOR_LOOP);
    EM::FileContent file_content{file_content_10K};
    const auto sections = LocateDocumentSections(file_content);
    auto financial_content = FindFinancialContentUsingAnchors(&sections, FILE_WITH_ANCHOR_LOOP);
    EXPECT_TRUE(financial_content);

    AnchorsFromHTML anchors(financial_content->first);

    static const boost::regex regex_balance_sheet{R"***((?:balance\s+sheet)|(?:financial.*?position))***",
                                                  boost::regex_constants::normal | boost::regex_constants::icase};
    auto balance_sheet_href =
        rng::find_if(anchors, [](const auto &anchor) { return AnchorFilterUsingRegex(regex_balance_sheet, anchor); });

    static const boost::regex regex_operations{R"***((?:statement|statements)\s+?of.*?(?:oper|loss|income|earning))***",
                                               boost::regex_constants::normal | boost::regex_constants::icase};

    auto stmt_of_ops_href =
        rng::find_if(anchors, [](const auto &anchor) { return AnchorFilterUsingRegex(regex_operations, anchor); });

    static const boost::regex regex_cash_flow{R"***((?:cash\s+flow)|(?:statement.+?cash)|(?:cashflow))***",
                                              boost::regex_constants::normal | boost::regex_constants::icase};

    auto cash_flows_href =
        rng::find_if(anchors, [](const auto &anchor) { return AnchorFilterUsingRegex(regex_cash_flow, anchor); });

    //    auto sholder_equity_href = std::find_if(anchors.begin(), anchors.end(),
    //    StockholdersEquityAnchorFilter);

    EXPECT_TRUE(balance_sheet_href != anchors.end());
    EXPECT_TRUE(stmt_of_ops_href != anchors.end());
    EXPECT_TRUE(cash_flows_href != anchors.end());

    auto balance_sheet_dest = FindDestinationAnchor(*balance_sheet_href, anchors);
    auto stmt_of_ops_dest = FindDestinationAnchor(*stmt_of_ops_href, anchors);
    auto cash_flows_dest = FindDestinationAnchor(*cash_flows_href, anchors);

    EXPECT_TRUE(balance_sheet_dest != anchors.end());
    EXPECT_TRUE(stmt_of_ops_dest != anchors.end());
    EXPECT_TRUE(cash_flows_dest != anchors.end());
}

class NoAnchors_10Q : public Test
{
};

TEST_F(NoAnchors_10Q, FindContentInFileWithNoAnchors1)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    auto financial_content = FindFinancialContentUsingAnchors(&sections, FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS);
    EXPECT_FALSE(financial_content);

    // let's see if we can find our data anyways

    HTML_FromFile htmls{&sections, FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS};

    FinancialStatements financial_statements;

    FinancialDocumentFilter document_filter{{"10-Q"}};
    for (auto html : htmls)
    {
        if (document_filter(html))
        {
            financial_statements = ExtractFinancialStatements(html.html_);
            if (financial_statements.has_data())
            {
                break;
            }
        }
    }

    //    std::cout << "\n\nBalance Sheet\n";
    //    std::cout.write(the_tables.balance_sheet_.the_data_.data(), 500);
    //    std::cout << "\n\nCash Flow\n";
    //    std::cout.write(the_tables.cash_flows_.the_data_.data(), 500);
    //    std::cout << "\n\nStmt of Operations\n";
    //    std::cout.write(the_tables.statement_of_operations_.the_data_.data(),
    //    500); std::cout << "\n\nShareholder Equity\n";
    //    std::cout.write(the_tables.stockholders_equity_.the_data_.data(),
    //            std::min(500UL,
    //            the_tables.stockholders_equity_.the_data_.size()));

    ASSERT_TRUE(financial_statements.has_data());
}

TEST_F(NoAnchors_10Q, FileWithNoAnchors2)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS2);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    // let's see if we can find our data anyways

    //    if (financial_content.empty())
    //    {
    //        financial_content = file_content_10Q;
    //    }
    //
    //    AnchorList statement_anchors;
    //
    //    AnchorsFromHTML anchors(financial_content);
    //
    //    std::copy_if(anchors.begin(),
    //            anchors.end(),
    //            std::back_inserter(statement_anchors),
    //            FinancialAnchorFilter
    //            );
    //
    //    EXPECT_TRUE(statement_anchors.size() < 3);

    const SharesOutstanding so;
    auto the_tables = FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS2);

    ASSERT_TRUE(the_tables.has_data());
}

TEST_F(NoAnchors_10Q, FileWithNoAnchors3)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS3);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    // let's see if we can find our data anyways

    //    if (financial_content.empty())
    //    {
    //        financial_content = file_content_10Q;
    //    }
    //
    //    AnchorList statement_anchors;
    //
    //    AnchorsFromHTML anchors(financial_content);
    //
    //    std::copy_if(anchors.begin(),
    //            anchors.end(),
    //            std::back_inserter(statement_anchors),
    //            FinancialAnchorFilter
    //            );
    //
    //    EXPECT_TRUE(statement_anchors.size() < 3);

    const SharesOutstanding so;
    auto the_tables = FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS3);

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(the_tables.balance_sheet_.parsed_data_.data(), 500);

    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(the_tables.statement_of_operations_.parsed_data_.data(), 500);

    std::cout << "\n\nCash Flow\n";
    std::cout.write(the_tables.cash_flows_.parsed_data_.data(), 500);

    std::cout << "\n\nShareholder Equity\n";
    std::cout.write(the_tables.stockholders_equity_.parsed_data_.data(),
                    std::min(500UL, the_tables.stockholders_equity_.parsed_data_.size()));

    ASSERT_TRUE(the_tables.has_data());
}

class ProblemWithRegexs_10Q : public Test
{
};

TEST_F(ProblemWithRegexs_10Q, UseRegexProblemFile1)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_PROBLEM_REGEX1);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    const SharesOutstanding so;
    auto the_tables = FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_HTML_10Q_PROBLEM_REGEX1);

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(the_tables.balance_sheet_.parsed_data_.data(), 500);

    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(the_tables.statement_of_operations_.parsed_data_.data(), 500);

    std::cout << "\n\nCash Flow\n";
    std::cout.write(the_tables.cash_flows_.parsed_data_.data(), 500);

    std::cout << "\n\nShareholder Equity\n";
    std::cout.write(the_tables.stockholders_equity_.parsed_data_.data(),
                    std::min(500UL, the_tables.stockholders_equity_.parsed_data_.size()));

    ASSERT_TRUE(the_tables.has_data());
}

TEST_F(ProblemWithRegexs_10Q, DISABLED_UseRegexProblemFile2)
{
    // Disabled because financial tables are split over multiple HTML tables and
    // this is not supported...yet.

    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_PROBLEM_REGEX2);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    const SharesOutstanding so;
    auto the_tables = FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_HTML_10Q_PROBLEM_REGEX2);

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(the_tables.balance_sheet_.parsed_data_.data(), 500);

    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(the_tables.statement_of_operations_.parsed_data_.data(), 500);

    std::cout << "\n\nCash Flow\n";
    std::cout.write(the_tables.cash_flows_.parsed_data_.data(), 500);

    std::cout << "\n\nShareholder Equity\n";
    std::cout.write(the_tables.stockholders_equity_.parsed_data_.data(),
                    std::min(500UL, the_tables.stockholders_equity_.parsed_data_.size()));

    ASSERT_TRUE(the_tables.has_data());
}

TEST_F(ProblemWithRegexs_10Q, ProblemMatchingCurrentAssets)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_XML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    const SharesOutstanding so;
    auto all_sections = FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_XML_10Q);

    EXPECT_TRUE(all_sections.has_data());

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);

    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(all_sections.statement_of_operations_.parsed_data_.data(), 500);

    std::cout << "\n\nCash Flow\n";
    std::cout.write(all_sections.cash_flows_.parsed_data_.data(), 500);

    //    for (const auto& [key, value] : all_sections.ListValues())
    //    {
    //        std::cout << "\nkey: " << key << " value: " << value << '\n';
    //    }
}

class ValidateCanNavigateDocumentStructure : public Test
{
};

// TEST_F(ValidateCanNavigateDocumentStructure, FindSECHeader_10K)
//{
//     auto file_content_10K = LoadXMLDataFileForUse(FILE_WITH_XML_10K);
//
//     SEC_Header SEC_data;
//
//     ASSERT_NO_THROW(SEC_data.UseData(file_content_10K));
// }
//
TEST_F(ValidateCanNavigateDocumentStructure, SECHeaderFindAllFields_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    SEC_Header SEC_data;
    SEC_data.UseData(file_content);
    ASSERT_NO_THROW(SEC_data.ExtractHeaderFields());
}

class ProcessEntireFileAndExtractData_10Q : public Test
{
public:
};

TEST_F(ProcessEntireFileAndExtractData_10Q, HTML_10Q_WITH_ANCHORS)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    const SharesOutstanding so;
    auto all_sections = FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_HTML_10Q_WITH_ANCHORS);

    EXPECT_TRUE(all_sections.has_data());

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);

    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(all_sections.statement_of_operations_.parsed_data_.data(), 500);

    std::cout << "\n\nCash Flow\n";
    std::cout.write(all_sections.cash_flows_.parsed_data_.data(), 500);

    std::cout << "\n\nShareholder Equity\n";
    std::cout.write(all_sections.stockholders_equity_.parsed_data_.data(),
                    std::min(500UL, all_sections.stockholders_equity_.parsed_data_.size()));

    //    for (const auto& [key, value] : all_sections.ListValues())
    //    {
    //        std::cout << "\nkey: " << key << " value: " << value << '\n';
    //    }
}

TEST_F(ProcessEntireFileAndExtractData_10Q, HTML_10Q_FIND_SHARES1)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_FIND_SHARES1);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    const SharesOutstanding so;
    auto all_sections = FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_HTML_10Q_FIND_SHARES1);
    EXPECT_TRUE(all_sections.has_data());

    int64_t shares = all_sections.outstanding_shares_;

    ASSERT_EQ(shares, 257360875);

    std::cout << "\n\nShares outstanding: " << shares << '\n';

    //    for (const auto& [key, value] : all_sections.ListValues())
    //    {
    //        std::cout << "\nkey: " << key << " value: " << value << '\n';
    //    }
    ASSERT_EQ(all_sections.ValuesTotal(), 76);
}

TEST_F(ProcessEntireFileAndExtractData_10Q, HTML_10Q_ASSETS_PROBLEM1)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_PROBLEM_WITH_ASSETS1);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    const SharesOutstanding so;
    auto all_sections =
        FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_HTML_10Q_PROBLEM_WITH_ASSETS1);

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);

    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(all_sections.statement_of_operations_.parsed_data_.data(), 500);

    std::cout << "\n\nCash Flow\n";
    std::cout.write(all_sections.cash_flows_.parsed_data_.data(), 500);

    std::cout << "\n\nShareholder Equity\n";
    std::cout.write(all_sections.stockholders_equity_.parsed_data_.data(),
                    std::min(500UL, all_sections.stockholders_equity_.parsed_data_.size()));
    //    for (const auto& [key, value] : all_sections.ListValues())
    //    {
    //        std::cout << "\nkey: " << key << " value: " << value << '\n';
    //    }
}

TEST_F(ProcessEntireFileAndExtractData_10Q, HTML10QBadCashFlowValue)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_BAD_CASH_FLOW_VALUE);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    const SharesOutstanding so;
    auto all_sections = FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_BAD_CASH_FLOW_VALUE);

    EXPECT_TRUE(all_sections.has_data());

    int64_t shares = all_sections.outstanding_shares_;

    EXPECT_EQ(shares, 673622066);

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);

    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(all_sections.statement_of_operations_.parsed_data_.data(), 500);

    std::cout << "\n\nCash Flow\n";
    std::cout.write(all_sections.cash_flows_.parsed_data_.data(), 500);

    std::cout << "\n\nShareholder Equity\n";
    std::cout.write(all_sections.stockholders_equity_.parsed_data_.data(),
                    std::min(500UL, all_sections.stockholders_equity_.parsed_data_.size()));
    //    for (const auto& [key, value] : all_sections.ListValues())
    //    {
    //        std::cout << "\nkey: " << key << " value: " << value << '\n';
    //    }
    ASSERT_EQ(all_sections.ValuesTotal(), 64);
}

TEST_F(ProcessEntireFileAndExtractData_10Q, HTML_10Q_WITH_ANCHORS_Collect1)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    const SharesOutstanding so;
    auto all_sections = FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_HTML_10Q_WITH_ANCHORS);

    EXPECT_TRUE(all_sections.has_data());

    int64_t shares = all_sections.outstanding_shares_;

    EXPECT_EQ(shares, 9230343);

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);

    //    for (const auto& [key, value] : all_sections.ListValues())
    //    {
    //        std::cout << "\nkey: " << key << " value: " << value << '\n';
    //    }
    ASSERT_EQ(all_sections.ValuesTotal(), 80);
}

TEST_F(ProcessEntireFileAndExtractData_10Q, XML_10Q_Collect1)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_XML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    const SharesOutstanding so;
    auto all_sections = FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_XML_10Q);

    EXPECT_TRUE(all_sections.has_data());

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);

    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(all_sections.statement_of_operations_.parsed_data_.data(), 500);

    //    for (const auto& [key, value] : all_sections.ListValues())
    //    {
    //        std::cout << "\nkey: " << key << " value: " << value << '\n';
    //    }
    std::cout << "\nFound: " << all_sections.ValuesTotal() << " values.\n";
    ASSERT_TRUE(all_sections.ValuesTotal() == 51);
}

TEST_F(ProcessEntireFileAndExtractData_10Q, XML_10Q_Collect2)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_XML2_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    const SharesOutstanding so;
    auto all_sections = FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_XML2_10Q);

    EXPECT_TRUE(all_sections.has_data());

    std::cout << "\n\nParsed Balance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);

    //    for (const auto& [key, value] : all_sections.ListValues())
    //    {
    //        std::cout << "\nkey: " << key << " value: " << value << '\n';
    //    }
    std::cout << "\nFound: " << all_sections.ValuesTotal() << " values.\n";
    ASSERT_TRUE(all_sections.ValuesTotal() == 59);
}

TEST_F(ProcessEntireFileAndExtractData_10Q, XML_10K_NoSharesOutstanding)
{
    const auto file_content_10K = LoadDataFileForUse(FILE_WITH_XBRL_INSIDE_TXT_DOCUMENT);
    EM::FileContent file_content{file_content_10K};
    const auto sections = LocateDocumentSections(file_content);

    const SharesOutstanding so;
    auto all_sections = FindAndExtractFinancialStatements(so, &sections, {"10-K"}, FILE_WITH_XBRL_INSIDE_TXT_DOCUMENT);
    EXPECT_FALSE(all_sections.has_data());

    int64_t shares = all_sections.outstanding_shares_;

    ASSERT_EQ(shares, -1);

    std::cout << "\n\nShares outstanding: " << shares << '\n';

    //    for (const auto& [key, value] : all_sections.ListValues())
    //    {
    //        std::cout << "\nkey: " << key << " value: " << value << '\n';
    //    }
}

TEST_F(ProcessEntireFileAndExtractData_10Q, HTML_NO_ANCHORS_10Q_Collect1)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS2);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);
    const SharesOutstanding so;
    auto all_sections =
        FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS2);

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);

    //    for (const auto& [key, value] : all_sections.ListValues())
    //    {
    //        std::cout << "\nkey: " << key << " value: " << value << '\n';
    //    }
    std::cout << "\nFound: " << all_sections.ValuesTotal() << " values.\n";
    ASSERT_TRUE(all_sections.ValuesTotal() == 28);
}

TEST_F(ProcessEntireFileAndExtractData_10Q, HTML_10Q_DUPLICATE_LABEL_TEXT)
{
    // this is mainly a visual test

    const auto file_content_10Q = LoadDataFileForUse(FILE_SHOWING_DUPLICATE_LABEL_TEXT);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    const SharesOutstanding so;
    auto all_sections = FindAndExtractFinancialStatements(so, &sections, {"10-Q"}, FILE_SHOWING_DUPLICATE_LABEL_TEXT);

    EXPECT_TRUE(all_sections.has_data());

    std::cout << "\n\nStmt of Operations\n";
    std::cout << all_sections.statement_of_operations_.parsed_data_ << '\n';

    for (const auto &[key, value] : all_sections.statement_of_operations_.values_)
    {
        std::cout << "\nkey: " << key << " value: " << value << '\n';
    }
}

class ProcessEntireFileAndExtractData_10K : public Test
{
public:
};

TEST_F(ProcessEntireFileAndExtractData_10K, XML_10K_Collect1)
{
    const auto file_content_10K = LoadDataFileForUse(FILE_WITH_HTML_NO_HREFS1_10K);
    EM::FileContent file_content{file_content_10K};
    const auto sections = LocateDocumentSections(file_content);
    const SharesOutstanding so;
    auto all_sections = FindAndExtractFinancialStatements(so, &sections, {"10-K"}, FILE_WITH_HTML_NO_HREFS1_10K);

    EXPECT_TRUE(all_sections.has_data());

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);

    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(all_sections.statement_of_operations_.parsed_data_.data(), 500);

    std::cout << "\n\nCash Flow\n";
    std::cout.write(all_sections.cash_flows_.parsed_data_.data(), 500);

    //    for (const auto& [key, value] : all_sections.ListValues())
    //    {
    //        std::cout << "\nkey: " << key << " value: " << value << '\n';
    //    }
    std::cout << "\nFound: " << all_sections.ValuesTotal() << " values.\n";
    ASSERT_TRUE(all_sections.ValuesTotal() == 97);
}

// the following tests are quite long since I have not encapsulated
// the required functionality.

class ExportHTML : public Test
{
public:
};

TEST_F(ExportHTML, HTML_10Q)
{
    if (!fs::exists("/tmp/export_html.txt"))
    {
        fs::remove("/tmp/export_html.txt");
    }
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    EM::FileContent file_content{file_content_10Q};
    const auto sections = LocateDocumentSections(file_content);

    HTML_FromFile htmls{&sections, FILE_WITH_HTML_10Q};

    FinancialDocumentFilter document_filter({"10-Q"});
    auto financial_content = std::find_if(std::begin(htmls), std::end(htmls), document_filter);
    EXPECT_TRUE(financial_content != htmls.end());

    SEC_Header SEC_data;
    SEC_data.UseData(file_content);

    auto file_header = SEC_data.GetHeader();

    // write content then read it back and parse it to complete test

    std::ofstream exported_file{"/tmp/export_html.txt"};
    exported_file.write(file_header.data(), file_header.size());
    exported_file.put('\n');
    exported_file.write(financial_content->document_.get().data(), financial_content->document_.get().size());
    exported_file.close();

    auto exported_data = LoadDataFileForUse(EM::FileName{"/tmp/export_html.txt"});
    EM::FileContent exported_content{exported_data};
    const auto exported_sections = LocateDocumentSections(exported_content);
    SEC_data.UseData(exported_content);

    HTML_FromFile htmls2{&exported_sections, EM::FileName{"/tmp/export_html.txt"}};
    auto reimported_content = std::find_if(std::begin(htmls2), std::end(htmls2), document_filter);
    ASSERT_TRUE(reimported_content != htmls2.end());
}

TEST_F(ExportHTML, VerifyExportAll_10Q)
{
    if (!fs::exists("/tmp/exported_html"))
    {
        fs::remove_all("/tmp/exported_html");
    }
    int files_with_form{0};

    auto hierarchy_converter =
        ConvertInputHierarchyToOutputHierarchy(SEC_DIRECTORY, EM::FileName{"/tmp/exported_html"});

    auto export_file([&files_with_form, &hierarchy_converter](const auto &dir_ent) {
        if (dir_ent.status().type() == fs::file_type::regular)
        {
            auto file_content_10Q = LoadDataFileForUse(EM::FileName{dir_ent.path().c_str()});
            EM::FileContent file_content{file_content_10Q};
            const auto sections = LocateDocumentSections(file_content);

            HTML_FromFile htmls{&sections, EM::FileName{dir_ent.path().c_str()}};

            for (const auto &html : htmls)
            {
                if (html.file_type_.get() != "10-Q")
                {
                    continue;
                }

                ++files_with_form;

                auto output_path_name =
                    hierarchy_converter(EM::FileName{dir_ent.path()}, html.file_name_.get().string());

                auto output_directory = output_path_name.parent_path();
                if (!fs::exists(output_directory))
                {
                    fs::create_directories(output_directory);
                }

                SEC_Header SEC_data;
                SEC_data.UseData(file_content);
                auto file_header = SEC_data.GetHeader();

                std::ofstream exported_file{output_path_name};
                exported_file.write(file_header.data(), file_header.size());
                exported_file.put('\n');
                exported_file.write(html.document_.get().data(), html.document_.get().size());
                exported_file.close();
                break;
            }
        }
    });
    std::for_each(fs::recursive_directory_iterator(SEC_DIRECTORY.get()), fs::recursive_directory_iterator(),
                  export_file);

    ASSERT_EQ(files_with_form, 186);
}

TEST_F(ExportHTML, VerifyCanProcessExportedHTML_10Q)
{
    if (!fs::exists("/tmp/exported_html"))
    {
        fs::remove_all("/tmp/exported_html");
    }
    int files_with_form{0};

    auto hierarchy_converter =
        ConvertInputHierarchyToOutputHierarchy(SEC_DIRECTORY, EM::FileName{"/tmp/exported_html"});

    auto export_file([&files_with_form, &hierarchy_converter](const auto &dir_ent) {
        if (dir_ent.status().type() == fs::file_type::regular)
        {
            auto file_content_10Q = LoadDataFileForUse(EM::FileName{dir_ent.path().c_str()});
            EM::FileContent file_content{file_content_10Q};
            const auto sections = LocateDocumentSections(file_content);

            HTML_FromFile htmls{&sections, EM::FileName{dir_ent.path().c_str()}};

            for (const auto &html : htmls)
            {
                if (html.file_type_.get() != "10-Q")
                {
                    continue;
                }

                ++files_with_form;

                auto output_path_name =
                    hierarchy_converter(EM::FileName{dir_ent.path()}, html.file_name_.get().string());

                auto output_directory = output_path_name.parent_path();
                if (!fs::exists(output_directory))
                {
                    fs::create_directories(output_directory);
                }

                SEC_Header SEC_data;
                SEC_data.UseData(file_content);
                auto file_header = SEC_data.GetHeader();

                std::ofstream exported_file{output_path_name};
                exported_file.write(file_header.data(), file_header.size());
                exported_file.put('\n');
                exported_file.write(html.document_.get().data(), html.document_.get().size());
                exported_file.close();
                break;
            }
        }
    });
    std::for_each(fs::recursive_directory_iterator(SEC_DIRECTORY.get()), fs::recursive_directory_iterator(),
                  export_file);

    EXPECT_EQ(files_with_form, 186);

    FinancialDocumentFilter document_filter({"10-Q"});
    int processed_files{0};

    auto process_exported_data([&document_filter, &processed_files](const auto &dir_ent) {
        if (dir_ent.status().type() == fs::file_type::regular)
        {
            auto file_content_10Q = LoadDataFileForUse(EM::FileName{dir_ent.path().c_str()});
            EM::FileContent file_content{file_content_10Q};
            const auto sections = LocateDocumentSections(file_content);

            HTML_FromFile htmls{&sections, EM::FileName{dir_ent.path().c_str()}};
            auto reimported_content = std::find_if(std::begin(htmls), std::end(htmls), document_filter);
            if (reimported_content != htmls.end())
            {
                ++processed_files;
            }
        }
    });

    std::for_each(fs::recursive_directory_iterator("/tmp/exported_html"), fs::recursive_directory_iterator(),
                  process_exported_data);

    ASSERT_EQ(processed_files, 186);
}

// we expect these tests to run against extracted HTML content meaning
// the relevant part of the original file has been isolated already.

class FindSharesOutstanding : public Test
{
public:
};

TEST_F(FindSharesOutstanding, Test10KFileWithSharesAndValue)
{
    const auto file_content_10K = LoadDataFileForUse(FILE_WITH_SHARES_AND_VALUE);
    EM::FileContent file_content{file_content_10K};
    const auto sections = LocateDocumentSections(file_content);

    HTML_FromFile htmls(&sections, FILE_WITH_SHARES_AND_VALUE);
    const SharesOutstanding so;

    int64_t found_shares = so(htmls.begin()->html_);

    ASSERT_EQ(found_shares, 191151089);
}

// ***
// the following disabled tests are due to no longer having the input
// files and not remembering how to create them.
//
// files may be on an old back drive....checking....slowly...
//
// ***
//
TEST_F(FindSharesOutstanding, DISABLED_Test10KFileWithNoDollarSignOnValue)
{
    const auto file_content_10K = LoadDataFileForUse(FILE_WITH_SHARE_VALUE_NO_DOLLAR_SIGN);
    EM::FileContent file_content{file_content_10K};
    const auto sections = LocateDocumentSections(file_content);

    HTML_FromFile htmls(&sections, FILE_WITH_SHARE_VALUE_NO_DOLLAR_SIGN);
    const SharesOutstanding so;

    int64_t found_shares = so(htmls.begin()->html_);

    ASSERT_EQ(found_shares, 706455305);
}

TEST_F(FindSharesOutstanding, DISABLED_Test10KFileWithDollarSignOnValue)
{
    const auto file_content_10K = LoadDataFileForUse(FILE_WITH_XBRL_INSIDE_HTM_DOCUMENT);
    EM::FileContent file_content{file_content_10K};
    const auto sections = LocateDocumentSections(file_content);

    HTML_FromFile htmls(&sections, FILE_WITH_XBRL_INSIDE_HTM_DOCUMENT);
    const SharesOutstanding so;

    int64_t found_shares = so(htmls.begin()->html_);
    //    ASSERT_THROW(so(htmls.begin()->html_), HTMLException);

    ASSERT_EQ(found_shares, -1);
}

TEST_F(FindSharesOutstanding, Test10KFileWithNoPossibles)
{
    const auto file_content_10K = LoadDataFileForUse(FILE_WITH_NO_POSSIBLES);
    EM::FileContent file_content{file_content_10K};
    const auto sections = LocateDocumentSections(file_content);

    HTML_FromFile htmls(&sections, FILE_WITH_NO_POSSIBLES);
    const SharesOutstanding so;

    int64_t found_shares = so(htmls.begin()->html_);

    ASSERT_EQ(found_shares, 169173941);
}

TEST_F(FindSharesOutstanding, DISABLED_Test10KFileWithSharesBeforeYesNo)
{
    const auto file_content_10K = LoadDataFileForUse(FILE_WITH_SHARES_BEFORE_YESNO);
    EM::FileContent file_content{file_content_10K};
    const auto sections = LocateDocumentSections(file_content);

    HTML_FromFile htmls(&sections, FILE_WITH_SHARES_BEFORE_YESNO);
    const SharesOutstanding so;

    int64_t found_shares = so(htmls.begin()->html_);

    ASSERT_EQ(found_shares, 2058165766);
}

TEST_F(FindSharesOutstanding, DISABLED_Test10KFileWithSharesAfterContents)
{
    const auto file_content_10K = LoadDataFileForUse(FILE_WITH_SHARES_AFTER_CONTENTS);
    EM::FileContent file_content{file_content_10K};
    const auto sections = LocateDocumentSections(file_content);

    HTML_FromFile htmls(&sections, FILE_WITH_SHARES_AFTER_CONTENTS);
    const SharesOutstanding so;

    int64_t found_shares = so(htmls.begin()->html_);

    ASSERT_EQ(found_shares, 2539442);
}

TEST_F(FindSharesOutstanding, TestListOfFilesWithAnswers)
{
    // tab delimited format.  file name, number of shares outstanding

    std::ifstream training_file(TRAINING_FILE_FOR_SHARES_OUTSTANDING.get());

    int success_ctr = 0;
    while (training_file.good())
    {
        std::string file_name;
        int64_t shares;
        training_file >> file_name;
        if (training_file.eof())
        {
            break;
        }
        training_file >> shares;

        std::cout << "\n\nProcessing file: " << file_name << '\n';

        auto content = LoadDataFileForUse(EM::FileName{file_name});
        EM::FileContent file_content{content};
        const auto sections = LocateDocumentSections(file_content);
        HTML_FromFile htmls(&sections, EM::FileName{file_name});

        SharesOutstanding so;
        int64_t found_shares = so(htmls.begin()->html_);
        if (found_shares == shares)
        {
            ++success_ctr;
        }
        std::cout << "Looking for: " << shares << " Found: " << found_shares
                  << ((found_shares == shares) ? " Success" : " Failure") << '\n';
    };

    ASSERT_EQ(success_ctr, 182);
}

/*
 * ===  FUNCTION
 * ====================================================================== Name:
 * InitLogging Description:
 * =====================================================================================
 */
void InitLogging()
{
    //    logging::core::get()->set_filter
    //    (
    //        logging::trivial::severity >= logging::trivial::trace
    //    );
    spdlog::set_level(spdlog::level::info);
} /* -----  end of function InitLogging  ----- */

int main(int argc, char **argv)
{

    InitLogging();

    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
