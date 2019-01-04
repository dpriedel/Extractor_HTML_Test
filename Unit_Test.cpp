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

    /* This file is part of ExtractEDGAR_HTML. */

    /* ExtractEDGAR_HTML is free software: you can redistribute it and/or modify */
    /* it under the terms of the GNU General Public License as published by */
    /* the Free Software Foundation, either version 3 of the License, or */
    /* (at your option) any later version. */

    /* ExtractEDGAR_HTML is distributed in the hope that it will be useful, */
    /* but WITHOUT ANY WARRANTY; without even the implied warranty of */
    /* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
    /* GNU General Public License for more details. */

    /* You should have received a copy of the GNU General Public License */
    /* along with ExtractEDGAR_HTML.  If not, see <http://www.gnu.org/licenses/>. */


// =====================================================================================
//        Class:
//  Description:
// =====================================================================================


#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <system_error>
#include <thread>

#include <boost/algorithm/string/predicate.hpp>

#include <gmock/gmock.h>

#include <range/v3/all.hpp>

#include "Poco/AutoPtr.h"
#include "Poco/Channel.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/Logger.h"
#include "Poco/Message.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/Application.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include <Poco/Net/NetException.h>
// #include "Poco/SimpleFileChannel.h"

namespace fs = std::filesystem;

using namespace testing;


using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;
using Poco::AutoPtr;

#include "EDGAR_HTML_FileFilter.h"
#include "ExtractEDGAR_Utils.h"
#include "HTML_FromFile.h"
#include "SEC_Header.h"
#include "AnchorsFromHTML.h"
#include "TablesFromFile.h"

// some specific files for Testing.

constexpr const char* FILE_WITH_HTML_10Q{"/vol_DA/EDGAR/Archives/edgar/data/1046672/0001102624-13-001243.txt"};
constexpr const char* FILE_WITH_XML_10Q{"/vol_DA/EDGAR/Archives/edgar/data/1460602/0001062993-13-005017.txt"};
constexpr const char* FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS{"/vol_DA/EDGAR/Archives/edgar/data/1046672/0001102624-13-001243.txt"};
constexpr const char* FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS2{"/vol_DA/EDGAR/Archives/edgar/data/841360/0001086380-13-000031.txt"};
constexpr const char* FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS3{"/vol_DA/EDGAR/Archives/edgar/data/949268/0001137171-09-000933.txt"};
constexpr const char* FILE_WITH_HTML_10Q_WITH_ANCHORS{"/vol_DA/EDGAR/Archives/edgar/data/1420525/0001420525-09-000028.txt"};
constexpr const char* FILE_WITH_HTML_10Q_WITH_ANCHORS2{"/vol_DA/EDGAR/Archives/edgar/data/1434743/0001193125-09-235665.txt"};
constexpr const char* FILE_WITH_HTML_10Q_WITH_ANCHORS3{"/vol_DA/EDGAR/Archives/edgar/data/1326688/0001104659-09-064933.txt"};
constexpr const char* FILE_WITH_HTML_10Q_WITH_ANCHORS4{"/vol_DA/EDGAR/Archives/edgar/data/1466739/0001002014-13-000458.txt"};
//constexpr const char* FILE_WITH_HTML_10Q_WITH_ANCHORS{"/tmp/x1.html"};
constexpr const char* FILE_WITH_XML_10K{"/vol_DA/EDGAR/Archives/edgar/data/google-10k.txt"};
constexpr const char* FILE_WITH_HTML_10Q_MINIMAL_DATA{"/vol_DA/EDGAR/Archives/edgar/data/841360/0001086380-13-000030.txt"};
constexpr const char* FILE_WITH_HTML_10Q_PROBLEM_REGEX1{"/vol_DA/EDGAR/Archives/edgar/data/1377936/0001104659-13-075719.txt"};
constexpr const char* FILE_WITH_HTML_10Q_PROBLEM_WITH_ASSETS1{"/vol_DA/EDGAR/Archives/edgar/data/68270/0000068270-13-000059.txt"};
constexpr const char* FILE_WITH_NO_HTML_10Q{"/vol_DA/EDGAR/Edgar_forms/855931/10-Q/0001130319-01-500242.txt"};
//constexpr const char* EDGAR_DIRECTORY{"/vol_DA/EDGAR/Archives/edgar/data"};
//constexpr const char* FILE_NO_NAMESPACE_10Q{"/vol_DA/EDGAR/Archives/edgar/data/68270/0000068270-13-000059.txt"};
//constexpr const char* FILE_SOME_NAMESPACE_10Q{"/vol_DA/EDGAR/Archives/edgar/data/1552979/0001214782-13-000386.txt"};
//constexpr const char* FILE_MULTIPLE_LABEL_LINKS{"/vol_DA/EDGAR/Archives/edgar/data/1540334/0001078782-13-002015.txt"};
//constexpr const char* BAD_FILE1{"/vol_DA/EDGAR/Edgar_forms/1000228/10-K/0001000228-11-000014.txt"};
//constexpr const char* BAD_FILE2{"/vol_DA/EDGAR/Edgar_forms/1000180/10-K/0001000180-16-000068.txt"};
//constexpr const char* BAD_FILE3{"/vol_DA/EDGAR/Edgar_forms/1000697/10-K/0000950123-11-018381.txt"};
//constexpr const char* NO_SHARES_OUT{"/vol_DA/EDGAR/Edgar_forms/1023453/10-K/0001144204-12-017368.txt"};
//constexpr const char* TEST_FILE_LIST{"./list_with_bad_file.txt"};
//constexpr const char* MISSING_VALUES1_10K{"/vol_DA/EDGAR/Edgar_forms/1004980/10-K/0001193125-12-065537.txt"};
//constexpr const char* MISSING_VALUES2_10K{"/vol_DA/EDGAR/Edgar_forms/1002638/10-K/0001193125-09-179839.txt"};

// This ctype facet does NOT classify spaces and tabs as whitespace
// from cppreference example

struct line_only_whitespace : std::ctype<char>
{
    static const mask* make_table()
    {
        // make a copy of the "C" locale table
        static std::vector<mask> v(classic_table(), classic_table() + table_size);
        v['\t'] &= ~space;      // tab will not be classified as whitespace
        v[' '] &= ~space;       // space will not be classified as whitespace
        return &v[0];
    }
    explicit line_only_whitespace(std::size_t refs = 0) : ctype(make_table(), false, refs) {}
};

// some utility functions for testing.


//  need these to feed into testing framework.

int G_ARGC = 0;
char** G_ARGV = nullptr;

Poco::Logger* THE_LOGGER = nullptr;

// using one of the example Poco programs to get going

class HTML_Extract_Unit_Test: public Application
    /// This sample demonstrates some of the features of the Util::Application class,
    /// such as configuration file handling and command line arguments processing.
    ///
    /// Try HTML_Extract_Unit_Test --help (on Unix platforms) or HTML_Extract_Unit_Test /help (elsewhere) for
    /// more information.
{
public:
    HTML_Extract_Unit_Test(): _helpRequested(false)
    {
    }

protected:
    void initialize(Application& self) override
    {
        loadConfiguration(); // load default configuration files, if present
        Application::initialize(self);
        // add your own initialization code here
    }

    void uninitialize() override
    {
        // add your own uninitialization code here
        Application::uninitialize();
    }

    void reinitialize(Application& self) override
    {
        Application::reinitialize(self);
        // add your own reinitialization code here
    }

    void defineOptions(OptionSet& options) override
    {
        Application::defineOptions(options);

        options.addOption(
            Option("help", "h", "display help information on command line arguments")
                .required(false)
                .repeatable(false)
                .callback(OptionCallback<HTML_Extract_Unit_Test>(this, &HTML_Extract_Unit_Test::handleHelp)));

        options.addOption(
            Option("gtest_filter", "", "select which tests to run.")
                .required(false)
                .repeatable(true)
                .argument("name=value")
                .callback(OptionCallback<HTML_Extract_Unit_Test>(this, &HTML_Extract_Unit_Test::handleDefine)));

        /* options.addOption( */
        /*  Option("define", "D", "define a configuration property") */
        /*      .required(false) */
        /*      .repeatable(true) */
        /*      .argument("name=value") */
        /*      .callback(OptionCallback<HTML_Extract_Unit_Test>(this, &HTML_Extract_Unit_Test::handleDefine))); */

        /* options.addOption( */
        /*  Option("config-file", "f", "load configuration data from a file") */
        /*      .required(false) */
        /*      .repeatable(true) */
        /*      .argument("file") */
        /*      .callback(OptionCallback<HTML_Extract_Unit_Test>(this, &HTML_Extract_Unit_Test::handleConfig))); */

        /* options.addOption( */
        /*  Option("bind", "b", "bind option value to test.property") */
        /*      .required(false) */
        /*      .repeatable(false) */
        /*      .argument("value") */
        /*      .binding("test.property")); */
    }

    void handleHelp(const std::string& name, const std::string& value)
    {
        _helpRequested = true;
        displayHelp();
        stopOptionsProcessing();
    }

    void handleDefine(const std::string& name, const std::string& value)
    {
        defineProperty(value);
    }

    void handleConfig(const std::string& name, const std::string& value)
    {
        loadConfiguration(value);
    }

    void displayHelp()
    {
        HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader("Test Driver application for ExtractEDGAR_XBRL.");
        helpFormatter.format(std::cout);
    }

    void defineProperty(const std::string& def)
    {
        std::string name;
        std::string value;
        std::string::size_type pos = def.find('=');
        if (pos != std::string::npos)
        {
            name.assign(def, 0, pos);
            value.assign(def, pos + 1, def.length() - pos);
        }
        else
        {
            name = def;
        }
        config().setString(name, value);
    }

    int main(const ArgVec& args) override
    {
        setLogger(*THE_LOGGER);
        if (!_helpRequested)
        {
            logger().information("Command line:");
            std::ostringstream ostr;
            for (ArgVec::const_iterator it = argv().begin(); it != argv().end(); ++it)
            {
                ostr << *it << ' ';
            }
            logger().information(ostr.str());
            logger().information("Arguments to main():");
            for (ArgVec::const_iterator it = args.begin(); it != args.end(); ++it)
            {
                logger().information(*it);
            }
            logger().information("Application properties:");
            printProperties("");

            //  run our tests

            testing::InitGoogleMock(&G_ARGC, G_ARGV);
            return RUN_ALL_TESTS();
        }
        return Application::EXIT_OK;
    }

    void printProperties(const std::string& base)
    {
        AbstractConfiguration::Keys keys;
        config().keys(base, keys);
        if (keys.empty())
        {
            if (config().hasProperty(base))
            {
                std::string msg;
                msg.append(base);
                msg.append(" = ");
                msg.append(config().getString(base));
                logger().information(msg);
            }
        }
        else
        {
            for (AbstractConfiguration::Keys::const_iterator it = keys.begin(); it != keys.end(); ++it)
            {
                std::string fullKey = base;
                if (!fullKey.empty()) fullKey += '.';
                fullKey.append(*it);
                printProperties(fullKey);
            }
        }
    }

private:
    bool _helpRequested;
};

class Iterators : public Test
{

};

TEST_F(Iterators, HTMLIteratorFileWithHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    HTML_FromFile html{file_content_10Q};

    auto how_many = std::distance(std::begin(html), std::end(html));
    EXPECT_TRUE(how_many == 5);

    auto htmls = Find_HTML_Documents(file_content_10Q);
    ASSERT_TRUE(htmls.size() == 5);
}

TEST_F(Iterators, HTMLIteratorFileWithMinimalHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_NO_HTML_10Q);
    HTML_FromFile html{file_content_10Q};

    auto how_many = std::distance(std::begin(html), std::end(html));

    EXPECT_TRUE(how_many == 0);

    auto htmls = Find_HTML_Documents(file_content_10Q);
    ASSERT_TRUE(htmls.size() == 0);
}

TEST_F(Iterators, HTMLIteratorFileWithHTML_10K)
{
    const auto file_content_10K = LoadDataFileForUse(FILE_WITH_XML_10K);
    HTML_FromFile html{file_content_10K};

    auto how_many = std::distance(std::begin(html), std::end(html));
    EXPECT_TRUE(how_many == 107);

    auto htmls = Find_HTML_Documents(file_content_10K);
    ASSERT_TRUE(htmls.size() == 107);
}

TEST_F(Iterators, AnchorIteratorFileWithHTML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    HTML_FromFile html{file_content_10Q};

    AnchorsFromHTML anchors(*html.begin());

    auto how_many = std::distance(std::begin(anchors), std::end(anchors));
    ASSERT_TRUE(how_many == 22);
}

TEST_F(Iterators, AnchorIteratorFileWithXML_10Q)
{
    const auto file_content_10Q = LoadDataFileForUse(FILE_WITH_XML_10Q);
    HTML_FromFile htmls{file_content_10Q};
    int total = 0;
    for (const auto& html : htmls)
    {
        AnchorsFromHTML anchors(html);
        total += std::distance(std::begin(anchors), std::end(anchors));
    }
    ASSERT_TRUE(total == 2215);
}

class IdentifyHTMLFilesToUse : public Test
{

};

TEST_F(IdentifyHTMLFilesToUse, ConfirmFileHasHTML)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);

    FileHasHTML filter1;
    auto use_file = filter1(EE::SEC_Header_fields{}, file_content_10Q);
    ASSERT_TRUE(use_file);
}

TEST_F(IdentifyHTMLFilesToUse, DISABLED_ConfirmFileHasXML)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_XML_10Q);

    FileHasHTML filter1;
    auto use_file = filter1(EE::SEC_Header_fields{}, file_content_10Q);
    ASSERT_FALSE(use_file);
}

class LocateDocumentWithFinancialContent : public Test
{

};

TEST_F(LocateDocumentWithFinancialContent, FileHasHTML_10Q)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    HTML_FromFile htmls{file_content_10Q};

    auto document = std::find_if(std::begin(htmls), std::end(htmls), FinancialDocumentFilter);
    ASSERT_TRUE(document != htmls.end());
}

TEST_F(LocateDocumentWithFinancialContent, FileHasXML_10Q)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_XML_10Q);
    HTML_FromFile htmls{file_content_10Q};

    auto document = std::find_if(std::begin(htmls), std::end(htmls), FinancialDocumentFilter);
    ASSERT_TRUE(document != htmls.end());
}

TEST_F(LocateDocumentWithFinancialContent, FileHasNoUsableAnchors_10Q)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS);
    HTML_FromFile htmls{file_content_10Q};

    auto document = std::find_if(std::begin(htmls), std::end(htmls), FinancialDocumentFilter);
    ASSERT_TRUE(document != htmls.end());
}

TEST_F(LocateDocumentWithFinancialContent, FileHasMinimalData_10Q)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_MINIMAL_DATA);
    HTML_FromFile htmls{file_content_10Q};

    auto document = std::find_if(std::begin(htmls), std::end(htmls), FinancialDocumentFilter);
    ASSERT_TRUE(document != htmls.end());
}

TEST_F(LocateDocumentWithFinancialContent, FileWithNoHTML_10Q)
{
    // this should find nothing because our iterator will be 'empty'

    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_NO_HTML_10Q);
    HTML_FromFile htmls{file_content_10Q};

    auto document = std::find_if(std::begin(htmls), std::end(htmls), FinancialDocumentFilter);
    ASSERT_TRUE(document == htmls.end());
}

//class FindAnchorsForFinancialStatements : public Test
//{
//public:
//};
//
//TEST_F(FindAnchorsForFinancialStatements, FindTopLevelAnchor_10Q)
//{
//    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS);
//
//    auto financial_content = FindFinancialContentUsingAnchors(file_content_10Q);
//
//    ASSERT_TRUE(! financial_content.empty());
//}
//
//TEST_F(FindAnchorsForFinancialStatements, FindTopLevelAnchorMinimalHTML_10Q)
//{
//    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_MINIMAL_DATA);
//
//    auto financial_content = FindFinancialContentUsingAnchors(file_content_10Q);
//
//    ASSERT_TRUE(! financial_content.empty());
//}
//
//TEST_F(FindAnchorsForFinancialStatements, FindTopLevelAnchorNoHTML_10Q)
//{
//    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_NO_HTML_10Q);
//
//    auto financial_content = FindFinancialContentUsingAnchors(file_content_10Q);
//
//    ASSERT_TRUE(financial_content.empty());
//}
//
//TEST_F(FindAnchorsForFinancialStatements, FindAnchors_10Q)
//{
//    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS);
//
//    AnchorList statement_anchors;
//
//    auto financial_content = FindFinancialContentUsingAnchors(file_content_10Q);
//    if (! financial_content.empty())
//    {
//        AnchorsFromHTML anchors(financial_content);
//        std::copy_if(anchors.begin(),
//                anchors.end(),
//                std::back_inserter(statement_anchors),
//                FinancialAnchorFilter
//                );
//    }
//    std::cout << "statement anchors: " << statement_anchors.size() << '\n';
//    ASSERT_TRUE(statement_anchors.size() == 4);
//}
//
//TEST_F(FindAnchorsForFinancialStatements, FindAnchorsMinimalHTML_10Q)
//{
//    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_MINIMAL_DATA);
//
//    AnchorList statement_anchors;
//
//    auto financial_content = FindFinancialContentUsingAnchors(file_content_10Q);
//    if (! financial_content.empty())
//    {
//        AnchorsFromHTML anchors(financial_content);
//        std::copy_if(anchors.begin(),
//                anchors.end(),
//                std::back_inserter(statement_anchors),
//                FinancialAnchorFilter
//                );
//    }
//    std::cout << "statement anchors: " << statement_anchors.size() << '\n';
//    ASSERT_TRUE(statement_anchors.size() == 3);
//}
//
//TEST_F(FindAnchorsForFinancialStatements, FindAnchorDestinations_10Q)
//{
//    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS);
//    documents = LocateDocumentSections(file_content_10Q);
//
//    auto all_anchors = FindAllDocumentAnchors(documents);
//    auto statement_anchors = FilterFinancialAnchors(all_anchors);
//    auto destination_anchors = FindAnchorDestinations(statement_anchors, all_anchors);
//
//    ASSERT_TRUE(destination_anchors.size() == 4);
//}
//
//TEST_F(FindAnchorsForFinancialStatements, FindAnchorDestinationsMinimalHTML_10Q)
//{
//    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_MINIMAL_DATA);
//    documents = LocateDocumentSections(file_content_10Q);
//
//    auto all_anchors = FindAllDocumentAnchors(documents);
//    auto statement_anchors = FilterFinancialAnchors(all_anchors);
//    auto destination_anchors = FindAnchorDestinations(statement_anchors, all_anchors);
//    std::cout << "\nDestination Anchors: \n";
//    for (const auto& anchor : destination_anchors)
//    {
//        std::cout
//            << "HREF: " << anchor.href
//            << "\tNAME: " << anchor.name
//            << "\tTEXT: " << anchor.text
//            << "\tCONTENT: " << anchor.anchor_content << '\n';
//    }
//
//    ASSERT_TRUE(destination_anchors.size() == 3);
//}
//
class FindIndividualFinancialStatements_10Q : public Test
{
public:

};

////TEST_F(FindIndividualFinancialStatements_10Q , FindDollarMultipliers_10Q)
////{
////    auto multipliers = FindDollarMultipliers(destination_anchors);
////
////    ASSERT_TRUE(multipliers.size() == 4);
////}
////
//TEST_F(FindIndividualFinancialStatements_10Q, FindTables_10Q)
//{
//    TablesFromHTML tables{financial_content};
//
//    auto how_many = std::distance(std::begin(tables), std::end(tables));
//
//    ASSERT_TRUE(how_many == 86);
//}
//
TEST_F(FindIndividualFinancialStatements_10Q, FindBalanceSheetFileWithHTML_10Q)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    auto document = FindFinancialDocument(file_content_10Q);

    TablesFromHTML tables{document};
    auto balance_sheet = std::find_if(tables.begin(), tables.end(), BalanceSheetFilter);

    ASSERT_TRUE(balance_sheet != tables.end());
}

TEST_F(FindIndividualFinancialStatements_10Q, FindStatementOfOperations_10Q)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    auto document = FindFinancialDocument(file_content_10Q);

    TablesFromHTML tables{document};
    auto statement_of_ops = std::find_if(tables.begin(), tables.end(), StatementOfOperationsFilter);

    ASSERT_TRUE(statement_of_ops != tables.end());
}

TEST_F(FindIndividualFinancialStatements_10Q, FindCashFlowStatement_10Q)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    auto document = FindFinancialDocument(file_content_10Q);

    TablesFromHTML tables{document};
    auto cash_flows = std::find_if(tables.begin(), tables.end(), CashFlowsFilter);

    ASSERT_TRUE(cash_flows != tables.end());
}

TEST_F(FindIndividualFinancialStatements_10Q, FindCashFlowStatement2_10Q)
{
    // use a different file here

    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS2);
    auto document = FindFinancialDocument(file_content_10Q);

    TablesFromHTML tables{document};
    auto cash_flows = std::find_if(tables.begin(), tables.end(), CashFlowsFilter);

    ASSERT_TRUE(cash_flows != tables.end());
    std::cout.write((*cash_flows).data(), 500);
}

TEST_F(FindIndividualFinancialStatements_10Q, FindStockholderEquity_10Q)
{
    // TODO: if can't find separate table, look into Balance sheet

    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);
    auto document = FindFinancialDocument(file_content_10Q);

    TablesFromHTML tables{document};
    auto stockholder_equity = std::find_if(tables.begin(), tables.end(), StockholdersEquityFilter);

    ASSERT_TRUE(stockholder_equity == tables.end());
}

class ProcessEntireFile_10Q : public Test
{
public:

};

TEST_F(ProcessEntireFile_10Q, ExtractAllNeededSections)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS);
    auto financial_content = FindFinancialDocument(file_content_10Q);

    auto all_sections = ExtractFinancialStatements(financial_content);

    ASSERT_TRUE(all_sections.has_data());
}

TEST_F(ProcessEntireFile_10Q, ExtractAllNeededSections2)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS2);
    auto financial_content = FindFinancialDocument(file_content_10Q);

    auto all_sections = ExtractFinancialStatements(financial_content);
    all_sections.PrepareTableContent();
    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);
    std::cout << "\n\nCash Flow\n";
    std::cout.write(all_sections.cash_flows_.parsed_data_.data(), 500);
    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(all_sections.statement_of_operations_.parsed_data_.data(), 500);
    std::cout << "\n\nShareholder Equity\n";
    std::cout.write(all_sections.stockholders_equity_.parsed_data_.data(), std::min(500UL, all_sections.stockholders_equity_.the_data_.size()));

    ASSERT_TRUE(all_sections.has_data());
}

//TEST_F(ProcessEntireFile_10Q, ExtractAllNeededSections3)
//{
//    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS3);
//    auto financial_content = FindFinancialContentUsingAnchors(file_content_10Q);
//
//    auto all_sections = ExtractFinancialStatements(financial_content);
//    std::cout << "\n\nBalance Sheet\n";
//    std::cout.write(all_sections.balance_sheet_.the_data_.data(), 500);
//    std::cout << "\n\nCash Flow\n";
//    std::cout.write(all_sections.cash_flows_.the_data_.data(), 500);
//    std::cout << "\n\nStmt of Operations\n";
//    std::cout.write(all_sections.statement_of_operations_.the_data_.data(), 500);
//    std::cout << "\n\nShareholder Equity\n";
//    std::cout.write(all_sections.stockholders_equity_.the_data_.data(), std::min(500UL, all_sections.stockholders_equity_.the_data_.size()));
//
//    ASSERT_TRUE(all_sections.has_data());
//}
//
//TEST_F(ProcessEntireFile_10Q, ExtractAllNeededSections4)
//{
//    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS4);
//    auto financial_content = FindFinancialContentUsingAnchors(file_content_10Q);
//
//    auto all_sections = ExtractFinancialStatements(financial_content);
//    std::cout << "\n\nBalance Sheet\n";
//    std::cout.write(all_sections.balance_sheet_.the_data_.data(), 500);
//    std::cout << "\n\nCash Flow\n";
//    std::cout.write(all_sections.cash_flows_.the_data_.data(), 500);
//    std::cout << "\n\nStmt of Operations\n";
//    std::cout.write(all_sections.statement_of_operations_.the_data_.data(), 500);
//    std::cout << "\n\nShareholder Equity\n";
//    std::cout.write(all_sections.stockholders_equity_.the_data_.data(), std::min(500UL, all_sections.stockholders_equity_.the_data_.size()));
//
//    ASSERT_TRUE(all_sections.has_data());
//}
//
TEST_F(ProcessEntireFile_10Q, ExtractAllNeededSectionsMinimalHTMLData)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_MINIMAL_DATA);
    auto financial_content = FindFinancialDocument(file_content_10Q);

    auto all_sections = ExtractFinancialStatements(financial_content);

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.the_data_.data(), 500);
    std::cout << "\n\nCash Flow\n";
    std::cout.write(all_sections.cash_flows_.the_data_.data(), 500);
    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(all_sections.statement_of_operations_.the_data_.data(), 500);
    std::cout << "\n\nShareholder Equity\n";
    std::cout.write(all_sections.stockholders_equity_.the_data_.data(), std::min(500UL, all_sections.stockholders_equity_.the_data_.size()));
    ASSERT_TRUE(all_sections.has_data());
}

class ProblemFiles_10Q : public Test
{

};

//TEST_F(ProblemFiles_10Q, FindAnchors_10Q)
//{
//    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS);
//
//    auto financial_content = FindFinancialContentUsingAnchors(file_content_10Q);
//
//    AnchorList statement_anchors;
//
//    if (! financial_content.empty())
//    {
//        AnchorsFromHTML anchors(financial_content);
//        std::cout << "statement anchors: " << std::distance(anchors.begin(), anchors.end()) << '\n';
////        for (const auto& anchor : anchors)
////        {
////            std::cout << anchor.href << '\t' << anchor.name << '\t' << anchor.text << '\t' << anchor.anchor_content << '\n';
////        }
//        std::copy_if(anchors.begin(),
//                anchors.end(),
//                std::back_inserter(statement_anchors),
//                FinancialAnchorFilter
//                );
//
//        std::cout << "\nFinancial anchors: \n";
//        for (const auto& anchor : statement_anchors)
//        {
//            std::cout << anchor.href << '\t' << anchor.name << '\t' << anchor.text << '\t' << anchor.anchor_content << '\n';
//        }
//    }
//    ASSERT_TRUE(statement_anchors.size() == 4);
//}
//
TEST_F(ProblemFiles_10Q, FileWithMinimalData)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_MINIMAL_DATA);

    auto financial_content = FindFinancialDocument(file_content_10Q);

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
//    EXPECT_TRUE(statement_anchors.size() == 3);
//
    TablesFromHTML tables{financial_content};

    auto balance_sheet = std::find_if(tables.begin(), tables.end(), BalanceSheetFilter);
    EXPECT_TRUE(balance_sheet != tables.end());

    auto statement_of_ops = std::find_if(tables.begin(), tables.end(), StatementOfOperationsFilter);
    EXPECT_TRUE(statement_of_ops != tables.end());

    auto cash_flows = std::find_if(tables.begin(), tables.end(), CashFlowsFilter);
    EXPECT_TRUE(cash_flows != tables.end());

    auto stockholder_equity = std::find_if(tables.begin(), tables.end(), StockholdersEquityFilter);
    EXPECT_TRUE(stockholder_equity == tables.end());
}

class NoAnchors_10Q : public Test
{

};

TEST_F(NoAnchors_10Q, FileWithNoAnchors1)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS);
    auto financial_content = FindFinancialDocument(file_content_10Q);
    EXPECT_TRUE(! financial_content.empty());
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

    TablesFromHTML tables{financial_content};
    auto how_many_tables = std::distance(tables.begin(), tables.end());
//    std::cout << "how_many_tables: " << how_many_tables << '\n';
    EXPECT_TRUE(how_many_tables == 68);

    auto balance_sheet = std::find_if(tables.begin(), tables.end(), BalanceSheetFilter);
    EXPECT_TRUE(balance_sheet != tables.end());

    auto statement_of_ops = std::find_if(tables.begin(), tables.end(), StatementOfOperationsFilter);
    EXPECT_TRUE(statement_of_ops != tables.end());

    auto cash_flows = std::find_if(tables.begin(), tables.end(), CashFlowsFilter);
    EXPECT_TRUE(cash_flows != tables.end());

    auto stockholder_equity = std::find_if(tables.begin(), tables.end(), StockholdersEquityFilter);
    EXPECT_TRUE(stockholder_equity == tables.end());

    auto the_tables = ExtractFinancialStatements(financial_content);

//    std::cout << "\n\nBalance Sheet\n";
//    std::cout.write(the_tables.balance_sheet_.the_data_.data(), 500);
//    std::cout << "\n\nCash Flow\n";
//    std::cout.write(the_tables.cash_flows_.the_data_.data(), 500);
//    std::cout << "\n\nStmt of Operations\n";
//    std::cout.write(the_tables.statement_of_operations_.the_data_.data(), 500);
//    std::cout << "\n\nShareholder Equity\n";
//    std::cout.write(the_tables.stockholders_equity_.the_data_.data(),
//            std::min(500UL, the_tables.stockholders_equity_.the_data_.size()));

    ASSERT_TRUE(the_tables.has_data());
}

TEST_F(NoAnchors_10Q, FileWithNoAnchors2)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS2);
    auto financial_content = FindFinancialDocument(file_content_10Q);
    EXPECT_TRUE(! financial_content.empty());

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

    auto the_tables = ExtractFinancialStatements(financial_content);

    ASSERT_TRUE(the_tables.has_data());
}

TEST_F(NoAnchors_10Q, FileWithNoAnchors3)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_NO_USABLE_ANCHORS3);
    auto financial_content = FindFinancialDocument(file_content_10Q);
    EXPECT_TRUE(! financial_content.empty());
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

    auto the_tables = ExtractFinancialStatements(financial_content);

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(the_tables.balance_sheet_.the_data_.data(), 500);
    
    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(the_tables.statement_of_operations_.the_data_.data(), 500);
    
    std::cout << "\n\nCash Flow\n";
    std::cout.write(the_tables.cash_flows_.the_data_.data(), 500);
    
    std::cout << "\n\nShareholder Equity\n";
    std::cout.write(the_tables.stockholders_equity_.the_data_.data(),
            std::min(500UL, the_tables.stockholders_equity_.the_data_.size()));

    ASSERT_TRUE(the_tables.has_data());
}

class ProblemWithRegexs_10Q : public Test
{

};

TEST_F(ProblemWithRegexs_10Q, UseRegexProblemFile1)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_PROBLEM_REGEX1);
    auto financial_content = FindFinancialDocument(file_content_10Q);
    // let's see if we can find our data anyways

//    if (financial_content.empty())
//    {
//        financial_content = file_content_10Q;
//    }

    auto the_tables = ExtractFinancialStatements(financial_content);

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(the_tables.balance_sheet_.the_data_.data(), 500);
    
    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(the_tables.statement_of_operations_.the_data_.data(), 500);
    
    std::cout << "\n\nCash Flow\n";
    std::cout.write(the_tables.cash_flows_.the_data_.data(), 500);
    
    std::cout << "\n\nShareholder Equity\n";
    std::cout.write(the_tables.stockholders_equity_.the_data_.data(),
            std::min(500UL, the_tables.stockholders_equity_.the_data_.size()));

    ASSERT_TRUE(the_tables.has_data());
}

TEST_F(ProblemWithRegexs_10Q, ProblemMatchingCurrentAssets)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_XML_10Q);
    auto financial_content = FindFinancialDocument(file_content_10Q);

    auto all_sections = ExtractFinancialStatements(financial_content);

    EXPECT_TRUE(all_sections.has_data());
    all_sections.PrepareTableContent();

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);

    EE::EDGAR_Labels extracted_data;
    ASSERT_NO_THROW(extracted_data = all_sections.CollectValues());

    for (const auto& [key, value] : extracted_data)
    {
        std::cout << "\nkey: " << key << " value: " << value << '\n';
    }
}

class ValidateCanNavigateDocumentStructure : public Test
{
};

//////TEST_F(ValidateCanNavigateDocumentStructure, FindSECHeader_10K)
//////{
//////    auto file_content_10K = LoadXMLDataFileForUse(FILE_WITH_XML_10K);
//////
//////    SEC_Header SEC_data;
//////
//////    ASSERT_NO_THROW(SEC_data.UseData(file_content_10K));
//////}
//////
TEST_F(ValidateCanNavigateDocumentStructure, SECHeaderFindAllFields_10Q)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q);

    SEC_Header SEC_data;
    SEC_data.UseData(file_content_10Q);
    ASSERT_NO_THROW(SEC_data.ExtractHeaderFields());
}

class ProcessEntireFileAndExtractData_10Q : public Test
{
public:

};

TEST_F(ProcessEntireFileAndExtractData_10Q, HTML_10Q_WITH_ANCHORS)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_WITH_ANCHORS);
    auto financial_content = FindFinancialDocument(file_content_10Q);

    auto all_sections = ExtractFinancialStatements(financial_content);

    EXPECT_TRUE(all_sections.has_data());

    all_sections.PrepareTableContent();

    EE::EDGAR_Labels extracted_data;
    ASSERT_NO_THROW(extracted_data = all_sections.CollectValues());

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);
    
    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(all_sections.statement_of_operations_.parsed_data_.data(), 500);
    
    std::cout << "\n\nCash Flow\n";
    std::cout.write(all_sections.cash_flows_.parsed_data_.data(), 500);
    
    std::cout << "\n\nShareholder Equity\n";
    std::cout.write(all_sections.stockholders_equity_.parsed_data_.data(),
            std::min(500UL, all_sections.stockholders_equity_.parsed_data_.size()));

    for (const auto& [key, value] : extracted_data)
    {
        std::cout << "\nkey: " << key << " value: " << value << '\n';
    }
}

TEST_F(ProcessEntireFileAndExtractData_10Q, HTML_10Q_ASSETS_PROBLEM1)
{
    auto file_content_10Q = LoadDataFileForUse(FILE_WITH_HTML_10Q_PROBLEM_WITH_ASSETS1);
    auto financial_content = FindFinancialDocument(file_content_10Q);

    auto all_sections = ExtractFinancialStatements(financial_content);

    EXPECT_TRUE(all_sections.has_data());

    all_sections.PrepareTableContent();

    std::cout << "\n\nBalance Sheet\n";
    std::cout.write(all_sections.balance_sheet_.parsed_data_.data(), 500);
    
    std::cout << "\n\nStmt of Operations\n";
    std::cout.write(all_sections.statement_of_operations_.parsed_data_.data(), 500);
    
    std::cout << "\n\nCash Flow\n";
    std::cout.write(all_sections.cash_flows_.parsed_data_.data(), 500);
    
    std::cout << "\n\nShareholder Equity\n";
    std::cout.write(all_sections.stockholders_equity_.parsed_data_.data(),
            std::min(500UL, all_sections.stockholders_equity_.parsed_data_.size()));
    all_sections.PrepareTableContent();

    EE::EDGAR_Labels extracted_data = all_sections.CollectValues();
    for (const auto& [key, value] : extracted_data)
    {
        std::cout << "\nkey: " << key << " value: " << value << '\n';
    }

}

int main(int argc, char** argv)
{
    G_ARGC = argc;
    G_ARGV = argv;

    int result = 0;

    HTML_Extract_Unit_Test the_app;
    try
    {
        the_app.init(argc, argv);

        THE_LOGGER = &Poco::Logger::get("TestLogger");
        AutoPtr<Poco::Channel> pChannel(new Poco::ConsoleChannel);
        // pChannel->setProperty("path", "/tmp/Testing.log");
        THE_LOGGER->setChannel(pChannel);
        THE_LOGGER->setLevel(Poco::Message::PRIO_DEBUG);

        result = the_app.run();
    }
    catch (Poco::Exception& exc)
    {
        the_app.logger().log(exc);
        result =  Application::EXIT_CONFIG;
    }

    return result;
}
