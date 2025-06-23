// =====================================================================================
//
//       Filename:  EndToEndTest.cpp
//
//    Description:  Driver program for end-to-end tests
//
//        Version:  1.0
//        Created:  01/03/2014 11:13:53 AM
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

// =====================================================================================
//        Class:
//  Description:
// =====================================================================================

#include <filesystem>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "ExtractorApp.h"
namespace fs = std::filesystem;

#include <gmock/gmock.h>
#include <spdlog/spdlog.h>

#include <pqxx/pqxx>

const fs::path FILE_WITH_XML_10Q{"/vol_DA/SEC/Archives/edgar/data/1460602/0001062993-13-005017.txt"};
const fs::path FILE_WITH_XML_10K{"/vol_DA/SEC/Archives/edgar/data/google-10k.txt"};
const fs::path FILE_WITHOUT_XML{"/vol_DA/SEC/Archives/edgar/data/841360/0001086380-13-000030.txt"};
const fs::path SEC_DIRECTORY{"/vol_DA/SEC/Archives/edgar/data"};
const fs::path FILE_NO_NAMESPACE_10Q{"/vol_DA/SEC/Archives/edgar/data/68270/0000068270-13-000059.txt"};
const fs::path BAD_FILE2{"/vol_DA/SEC/SEC_forms/1000180/10-K/0001000180-16-000068.txt"};
const fs::path NO_SHARES_OUT{"/vol_DA/SEC/SEC_forms/1023453/10-K/0001144204-12-017368.txt"};
const fs::path MISSING_VALUES_LIST{"../Extractor_XBRL_Test/missing_values_files.txt"};
const fs::path CRASH_IN_REGEX{"/vol_DA/SEC/Archives/edgar/data/1060409/0001211524-13-000254.txt"};

constexpr const char *FILE_WITH_HTML_10Q_WITH_ANCHORS{
    "/vol_DA/SEC/Archives/edgar/data/1420525/0001420525-09-000028.txt"};

using namespace testing;

int CountFilesInDirectoryTree(const fs::path &directory)
{
    int count =
        std::count_if(fs::recursive_directory_iterator(directory), fs::recursive_directory_iterator(),
                      [](const fs::directory_entry &entry) { return entry.status().type() == fs::file_type::regular; });
    return count;
}

std::map<std::string, fs::file_time_type> CollectLastModifiedTimesForFilesInDirectoryTree(const fs::path &directory)
{
    std::map<std::string, fs::file_time_type> results;

    auto save_mod_time([&results](const auto &dir_ent) {
        if (dir_ent.status().type() == fs::file_type::regular)
        {
            results[dir_ent.path().filename().string()] = fs::last_write_time(dir_ent.path());
        }
    });

    std::for_each(fs::recursive_directory_iterator(directory), fs::recursive_directory_iterator(), save_mod_time);

    return results;
}

class SingleFileEndToEndXBRL : public Test
{
protected:
    void SetUp() override
    {
        pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
        pqxx::work trxn{c};

        // make sure the DB is empty before we start

        trxn.exec("DELETE FROM unified_extracts.sec_filing_id WHERE data_source != 'HTML'");
        trxn.commit();
    }

public:
    int CountRows()
    {
        pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
        pqxx::work trxn{c};

        auto row1 = trxn.query_value<int>("select count(*) from unified_extracts.sec_filing_id as t1 inner join "
                                          "unified_extracts.sec_bal_sheet_data as t2 on t1.filing_id =  "
                                          "t2.filing_id where t1.data_source = 'XLS';");
        auto row2 = trxn.query_value<int>("select count(*) from unified_extracts.sec_filing_id as t1 inner join "
                                          "unified_extracts.sec_stmt_of_ops_data as t2 on t1.filing_id =  "
                                          "t2.filing_id where t1.data_source = 'XLS';");
        auto row3 = trxn.query_value<int>("select count(*) from unified_extracts.sec_filing_id as t1 inner join "
                                          "unified_extracts.sec_cash_flows_data as t2 on t1.filing_id =  "
                                          "t2.filing_id where t1.data_source = 'XLS';");
        trxn.commit();
        int total = row1 + row2 + row3;
        if (total == 0)
        {
            // maybe we have plain XBRL

            pqxx::work trxn{c};
            total = trxn.query_value<int>("select count(*) from unified_extracts.sec_filing_id as t1 inner "
                                          "join unified_extracts.sec_xbrl_data as t2 on t1.filing_id =  "
                                          "t2.filing_id where t1.data_source = 'XBRL';");
            trxn.commit();
        }
        return total;
    }
};

TEST_F(SingleFileEndToEndXBRL, VerifyCanLoadDataToDBForFileWithXML10QXBRL)
{
    //	NOTE: the program name 'the_program' in the command line below is
    // ignored in the 	the test program.

    // NOTE:  use 'replace-DB-content' option because the SetUp function appears to
    // be incorrect -- I get duplicate key errors *somethimes*. Since this test
    // is not about replacement processing, just do it.

    std::vector<std::string> tokens{"the_program",
                                    "--log-level",
                                    "debug",
                                    "--form",
                                    "10-Q",
                                    "--mode",
                                    "XBRL",
                                    "-f",
                                    FILE_WITH_XML_10Q.string(),
                                    "--replace-DB-content",
                                    "--log-path",
                                    "/tmp/Extractor/test01.log"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_EQ(CountRows(), 55);
}

TEST_F(SingleFileEndToEndXBRL, VerifyLoadsNoDataToDBForFileWithXML10QHTML)
{
    //	NOTE: the program name 'the_program' in the command line below is
    // ignored in the 	the test program.

    std::vector<std::string> tokens{"the_program",
                                    "--log-level",
                                    "information",
                                    "--form",
                                    "10-Q",
                                    "--mode",
                                    "HTML",
                                    "-f",
                                    FILE_WITH_XML_10Q.string(),
                                    "--log-path",
                                    "/tmp/Extractor/test02.log"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_EQ(CountRows(), 0);
}

class SingleFileEndToEndHTML : public Test
{
protected:
    void SetUp() override
    {
        pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
        pqxx::work trxn{c};

        // make sure the DB is empty before we start

        trxn.exec("DELETE FROM unified_extracts.sec_filing_id WHERE data_source = "
                  "'HTML'");
        trxn.commit();
    }

public:
    int CountRows()
    {
        pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
        pqxx::work trxn{c};

        auto row1 = trxn.query_value<int>("select count(*) from unified_extracts.sec_filing_id as t1 inner join "
                                          "unified_extracts.sec_bal_sheet_data as t2 on t1.filing_id =  "
                                          "t2.filing_id where t1.data_source = 'HTML';");
        auto row2 = trxn.query_value<int>("select count(*) from unified_extracts.sec_filing_id as t1 inner join "
                                          "unified_extracts.sec_stmt_of_ops_data as t2 on t1.filing_id =  "
                                          "t2.filing_id where t1.data_source = 'HTML';");
        auto row3 = trxn.query_value<int>("select count(*) from unified_extracts.sec_filing_id as t1 inner join "
                                          "unified_extracts.sec_cash_flows_data as t2 on t1.filing_id =  "
                                          "t2.filing_id where t1.data_source = 'HTML';");
        trxn.commit();
        int total = row1 + row2 + row3;
        return total;
    }
};

TEST_F(SingleFileEndToEndHTML, VerifyCanLoadDataToDBForFileWithHTML10QHTML)
{
    //	NOTE: the program name 'the_program' in the command line below is
    // ignored in the 	the test program.

    std::vector<std::string> tokens{"the_program", "--log-level", "debug", "--form", "10-Q", "--mode", "HTML",
                                    //		"-f", CRASH_IN_REGEX
                                    "-f", FILE_WITH_HTML_10Q_WITH_ANCHORS, "--log-path", "/tmp/Extractor/test03.log"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }

    ASSERT_EQ(CountRows(), 80);
}

TEST_F(SingleFileEndToEndHTML, VerifyCanLoadDataToDBForFileWithXML10K)
{
    //	NOTE: the program name 'the_program' in the command line below is
    // ignored in the 	the test program.

    std::vector<std::string> tokens{"the_program",
                                    "--begin-date",
                                    "2013-10-14",
                                    "--end-date",
                                    "2016-12-30",
                                    "--log-level",
                                    "information",
                                    "--mode",
                                    "HTML",
                                    "--form",
                                    "10-K",
                                    "-f",
                                    FILE_WITH_XML_10K.string(),
                                    "--log-path",
                                    "/tmp/Extractor/test04.log"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }

    ASSERT_EQ(CountRows(), 101);
}

// TEST_F(SingleFileEndToEnd, WorkWithBadFile2_10K)
//{
//	//	NOTE: the program name 'the_program' in the command line below is
// is ignored in the
//	//	the test program.
//
//	std::vector<std::string> tokens{"the_program",
//         "--log-level", "information",
//		"--form", "10-K",
//		"-f", BAD_FILE2.string()
//	};
//
//     ExtractorApp myApp;
//	try
//	{
//         myApp.init(tokens);
//
//		decltype(auto) test_info =
// UnitTest::GetInstance()->current_test_info();
//		myApp.logger().information(std::string("\n\nTest: ") +
// test_info->name() + " test case: " + test_info->test_case_name() + "\n\n");
//
//         myApp.run();
//	}
//
//     // catch any problems trying to setup application
//
//	catch (const std::exception& theProblem)
//	{
//		// poco_fatal(myApp->logger(), theProblem.what());
//
//		myApp.logger().error(std::string("Something fundamental went
// wrong: ") + theProblem.what()); 		throw;	//	so test
// framework will get it too.
//	}
//	catch (...)
//	{		// handle exception: unspecified
//		myApp.logger().error("Something totally unexpected happened.");
//		throw;
//	}
//	ASSERT_EQ(CountRows(), 1668);
// }
//
class ProcessFolderEndtoEnd : public Test
{
protected:
    void SetUp() override
    {
        pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
        pqxx::work trxn{c};

        // make sure the DB is empty before we start

        trxn.exec("DELETE FROM unified_extracts.sec_filing_id WHERE data_source = 'HTML'");
        trxn.commit();
    }

public:
    int CountRows()
    {
        pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
        pqxx::work trxn{c};

        // make sure the DB is empty before we start

        auto row1 = trxn.exec("SELECT count(*) FROM unified_extracts.sec_bal_sheet_data").one_row();
        auto row2 = trxn.exec("SELECT count(*) FROM unified_extracts.sec_stmt_of_ops_data").one_row();
        auto row3 = trxn.exec("SELECT count(*) FROM unified_extracts.sec_cash_flows_data").one_row();
        trxn.commit();
        return row1[0].as<int>() + row2[0].as<int>() + row3[0].as<int>();
    }

    int CountMissingValues()
    {
        pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
        pqxx::work trxn{c};

        auto row1 = trxn.exec("SELECT count(*) FROM unified_extracts.sec_bal_sheet_data "
                              "WHERE label = 'Missing Value'")
                        .one_row();
        auto row2 = trxn.exec("SELECT count(*) FROM unified_extracts.sec_stmt_of_ops_data "
                              "WHERE label = 'Missing Value'")
                        .one_row();
        auto row3 = trxn.exec("SELECT count(*) FROM unified_extracts.sec_cash_flows_data "
                              "WHERE label = 'Missing Value'")
                        .one_row();
        trxn.commit();
        return row1[0].as<int>() + row2[0].as<int>() + row3[0].as<int>();
    }

    int CountFilings()
    {
        pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
        pqxx::work trxn{c};

        // make sure the DB is empty before we start

        auto row = trxn.exec("SELECT count(*) FROM unified_extracts.sec_filing_id "
                             "WHERE data_source = 'HTML'")
                       .one_row();
        trxn.commit();
        return row[0].as<int>();
    }
};

TEST_F(ProcessFolderEndtoEnd, UseDirectory10QHTML)
{
    //	NOTE: the program name 'the_program' in the command line below is
    // ignored in the 	the test program.

    std::vector<std::string> tokens{"the_program", "--log-level", "information", "--form", "10-Q", "--mode", "HTML",
                                    // "-k",
                                    // "1",
                                    "--form-dir", SEC_DIRECTORY.string(), "--log-path", "/tmp/Extractor/test05.log"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_EQ(CountFilings(), 153);
}

TEST_F(ProcessFolderEndtoEnd, WorkWithFileList3_10Q)
{
    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    std::vector<std::string> tokens{"the_program",
                                    "--mode",
                                    "HTML",
                                    "--log-level",
                                    "information",
                                    "--form",
                                    "10-Q",
                                    "--log-path",
                                    "/tmp/Extractor/test06.log",
                                    "--list",
                                    "./test_directory_list.txt"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_EQ(CountFilings(), 146);
}

TEST_F(ProcessFolderEndtoEnd, WorkWithFileListResume_10Q)
{
    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    std::vector<std::string> tokens{"the_program",
                                    "--log-level",
                                    "information",
                                    "--mode",
                                    "HTML",
                                    "--form",
                                    "10-Q",
                                    "--log-path",
                                    "/tmp/Extractor/test07.log",
                                    "--list",
                                    "./test_directory_list.txt",
                                    "--resume-at",
                                    "/vol_DA/SEC/Archives/edgar/data/1326688/0001104659-09-064933.txt"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_EQ(CountFilings(), 27);
}

TEST_F(ProcessFolderEndtoEnd, WorkWithFileList3Async10Q)
{
    //	NOTE: the program name 'the_program' in the command line below is
    // ignored in the 	the test program.

    std::vector<std::string> tokens{"the_program",
                                    "--log-level",
                                    "information",
                                    "--form",
                                    "10-Q,10-K",
                                    "--mode",
                                    "HTML",
                                    "-k",
                                    "6",
                                    "--list",
                                    "./test_directory_list.txt",
                                    "--log-path",
                                    "/tmp/Extractor/test08.log"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_EQ(CountFilings(), 147);
}

TEST_F(ProcessFolderEndtoEnd, WorkWithFileList3WithLimitAsync_10Q)
{
    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    std::vector<std::string> tokens{"the_program",
                                    "--log-level",
                                    "information",
                                    "--mode",
                                    "HTML",
                                    "--form",
                                    "10-Q",
                                    "--max",
                                    "17",
                                    "-k",
                                    "4",
                                    "--list",
                                    "./test_directory_list.txt",
                                    "--log-path",
                                    "/tmp/Extractor/test09.log"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_EQ(CountFilings(), 17);
}

TEST_F(ProcessFolderEndtoEnd, WorkWithFileList3_10K)
{
    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    std::vector<std::string> tokens{"the_program",
                                    "--log-level",
                                    "information",
                                    "--mode",
                                    "HTML",
                                    "--form",
                                    "10-K",
                                    "--list",
                                    "./test_directory_list.txt",
                                    "--log-path",
                                    "/tmp/Extractor/test10.log"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_EQ(CountFilings(), 1);
}

// TEST_F(ProcessFolderEndtoEnd, VerifyCanApplyFilters)
//{
//	//	NOTE: the program name 'the_program' in the command line below
// is ignored in the
//	//	the test program.
//
//	std::vector<std::string> tokens{"the_program",
//         "--log-level", "information",
//		"--form", "10-K",
//		"--form-dir", SEC_DIRECTORY.string()
//	};
//
//     ExtractorApp myApp;
//	try
//	{
//         myApp.init(tokens);
//
//		decltype(auto) test_info =
// UnitTest::GetInstance()->current_test_info();
//		myApp.logger().information(std::string("\n\nTest: ") +
// test_info->name() + " test case: " + test_info->test_case_name() + "\n\n");
//
//         myApp.run();
//	}
//
//     // catch any problems trying to setup application
//
//	catch (const std::exception& theProblem)
//	{
//		// poco_fatal(myApp->logger(), theProblem.what());
//
//		myApp.logger().error(std::string("Something fundamental went
// wrong: ") + theProblem.what()); 		throw;	//	so test
// framework will get it too.
//	}
//	catch (...)
//	{		// handle exception: unspecified
//		myApp.logger().error("Something totally unexpected happened.");
//		throw;
//	}
//	ASSERT_EQ(CountFilings(), 1);
// }
//
// TEST_F(ProcessFolderEndtoEnd, VerifyCanApplyFilters2)
//{
//	//	NOTE: the program name 'the_program' in the command line below
// is ignored in the
//	//	the test program.
//
//	std::vector<std::string> tokens{"the_program",
//		"--begin-date", "2013-Mar-1",
//		"--end-date", "2013-3-31",
//         "--log-level", "information",
//		"--form", "10-Q",
//		"--form-dir", SEC_DIRECTORY.string()
//	};
//
//     ExtractorApp myApp;
//	try
//	{
//         myApp.init(tokens);
//
//		decltype(auto) test_info =
// UnitTest::GetInstance()->current_test_info();
//		myApp.logger().information(std::string("\n\nTest: ") +
// test_info->name() + " test case: " + test_info->test_case_name() + "\n\n");
//
//         myApp.run();
//	}
//
//     // catch any problems trying to setup application
//
//	catch (const std::exception& theProblem)
//	{
//		// poco_fatal(myApp->logger(), theProblem.what());
//
//		myApp.logger().error(std::string("Something fundamental went
// wrong: ") + theProblem.what()); 		throw;	//	so test
// framework will get it too.
//	}
//	catch (...)
//	{		// handle exception: unspecified
//		myApp.logger().error("Something totally unexpected happened.");
//		throw;
//	}
//	ASSERT_EQ(CountFilings(), 5);
// }
//
TEST_F(ProcessFolderEndtoEnd, VerifyCanApplyFilters3)
{
    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    std::vector<std::string> tokens{"the_program",
                                    "--begin-date",
                                    "2013-Mar-1",
                                    "--end-date",
                                    "2013-3-31",
                                    "--log-level",
                                    "information",
                                    "--mode",
                                    "HTML",
                                    "--form",
                                    "10-K,10-Q",
                                    "--form-dir",
                                    SEC_DIRECTORY.string(),
                                    "--log-path",
                                    "/tmp/Extractor/test10_2.log"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_EQ(CountFilings(), 5);
}

TEST_F(ProcessFolderEndtoEnd, VerifyCanApplyFilters4ShortCIKFails)
{
    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    std::vector<std::string> tokens{"the_program",
                                    "--begin-date",
                                    "2013-Mar-1",
                                    "--end-date",
                                    "2013-3-31",
                                    "--log-level",
                                    "information",
                                    "--mode",
                                    "HTML",
                                    "--form",
                                    "10-K,10-Q",
                                    "--CIK",
                                    "1541884",
                                    "--form-dir",
                                    SEC_DIRECTORY.string(),
                                    "--log-path",
                                    "/tmp/Extractor/test10_4.log"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_EQ(CountFilings(), 0);
}

TEST_F(ProcessFolderEndtoEnd, VerifyCanApplyFilters5)
{
    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    std::vector<std::string> tokens{"the_program",
                                    "--begin-date",
                                    "2013-Mar-1",
                                    "--end-date",
                                    "2013-3-31",
                                    "--mode",
                                    "HTML",
                                    "--log-level",
                                    "information",
                                    "--form",
                                    "10-K,10-Q",
                                    "--CIK",
                                    "0000826772,0000826774",
                                    "--form-dir",
                                    SEC_DIRECTORY.string(),
                                    "--log-path",
                                    "/tmp/Extractor/test11.log"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_EQ(CountFilings(), 1);
}

// TEST_F(ProcessFolderEndtoEnd, LoadLotsOfFiles)
//{
//	//	NOTE: the program name 'the_program' in the command line below
// is ignored in the
//	//	the test program.
//
//	std::vector<std::string> tokens{"the_program",
//         "--log-level", "information",
//		"--form", "10-Q",
//		"--form-dir", SEC_DIRECTORY.string()
//	};
//
//     ExtractorApp myApp;
//	try
//	{
//         myApp.init(tokens);
//
//		decltype(auto) test_info =
// UnitTest::GetInstance()->current_test_info();
//		myApp.logger().information(std::string("\n\nTest: ") +
// test_info->name() + " test case: " + test_info->test_case_name() + "\n\n");
//
//         myApp.run();
//	}
//
//     // catch any problems trying to setup application
//
//	catch (const std::exception& theProblem)
//	{
//		// poco_fatal(myApp->logger(), theProblem.what());
//
//		myApp.logger().error(std::string("Something fundamental went
// wrong: ") + theProblem.what()); 		throw;	//	so test
// framework will get it too.
//	}
//	catch (...)
//	{		// handle exception: unspecified
//		myApp.logger().error("Something totally unexpected happened.");
//		throw;
//	}
//	// NOTE: there are 157 files which meet the scan criteria BUT 2 of them
// are duplicated. 	ASSERT_EQ(CountFilings(), 155);
// }
//
// TEST_F(ProcessFolderEndtoEnd, LoadLotsOfFilesWithLimit)
//{
//	//	NOTE: the program name 'the_program' in the command line below
// is ignored in the
//	//	the test program.
//
//	std::vector<std::string> tokens{"the_program",
//         "--log-level", "information",
//		"--form", "10-Q",
//		"--max", "14",
//		"--form-dir", SEC_DIRECTORY.string()
//	};
//
//     ExtractorApp myApp;
//	try
//	{
//         myApp.init(tokens);
//
//		decltype(auto) test_info =
// UnitTest::GetInstance()->current_test_info();
//		myApp.logger().information(std::string("\n\nTest: ") +
// test_info->name() + " test case: " + test_info->test_case_name() + "\n\n");
//
//         myApp.run();
//	}
//
//     // catch any problems trying to setup application
//
//	catch (const std::exception& theProblem)
//	{
//		// poco_fatal(myApp->logger(), theProblem.what());
//
//		myApp.logger().error(std::string("Something fundamental went
// wrong: ") + theProblem.what()); 		throw;	//	so test
// framework will get it too.
//	}
//	catch (...)
//	{		// handle exception: unspecified
//		myApp.logger().error("Something totally unexpected happened.");
//		throw;
//	}
//	// NOTE: there are 157 files which meet the scan criteria BUT 2 of them
// are duplicated. 	ASSERT_EQ(CountFilings(), 14);
// }
//
//// TEST(DailyEndToEndTest,
/// VerifyDownloadsOfExistingFormFilesWhenReplaceIsSpecifed) / { / 	if
///(fs::exists("/tmp/index2")) / 		fs::remove_all("/tmp/index2");
//// 	if (fs::exists("/tmp/forms2"))
//// 		fs::remove_all("/tmp/forms2");
//// 	fs::create_directory("/tmp/forms2");
////
//// 	//	NOTE: the program name 'the_program' in the command line below
/// is ignored in the / 	//	the test program.
////
//// 	std::vector<std::string> tokens{"the_program",
//// 		"--index-dir", "/tmp/index2",
//// 		"--form-dir", "/tmp/forms2",
////         "--host", "https://localhost:8443",
//// 		"--begin-date", "2013-Oct-14",
//// 		"--end-date", "2013-Oct-17",
//// 		"--replace-form-files"
//// 	};
////
//// 	ExtractorApp myApp;
////     myApp.init(tokens);
////
//// 	decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
//// 	myApp.logger().information(std::string("\n\nTest: ") + test_info->name()
///+ " test case: " + test_info->test_case_name() + "\n\n");
////
////     myApp.run();
//// 	// decltype(auto) x1 =
/// CollectLastModifiedTimesForFilesInDirectoryTree("/tmp/forms2");
////
//// 	std::this_thread::sleep_for(std::chrono::seconds{1});
////
//// 	myApp.run();
//// 	// decltype(auto) x2 =
/// CollectLastModifiedTimesForFilesInDirectoryTree("/tmp/forms2");
////
//// 	// ASSERT_THAT(x1 == x2, Eq(false));
//// 	ASSERT_TRUE(0);
//// }

class ExportHTML : public Test
{
public:
};

TEST_F(ExportHTML, ExportSingleFile)
{
    //	NOTE: the program name 'the_program' in the command line below is
    // ignored in the 	the test program.

    if (fs::exists("/tmp/extracts"))
        fs::remove_all("/tmp/extracts");

    std::vector<std::string> tokens{"the_program", "--log-level", "information", "--form", "10-Q,10-K", "--mode",
                                    "HTML", "--log-path", "/tmp/Extractor/test12.log",
                                    //		"--list", "./list_with_bad_file.txt"
                                    "--export-HTML-data", "--HTML-forms-to-dir", "/tmp/extracts",
                                    "--HTML-forms-from-dir", "/vol_DA/SEC/Archives/edgar", "--file",
                                    "/vol_DA/SEC/Archives/edgar/data/google-10k.txt"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_TRUE(fs::exists("/tmp/extracts/data/google-10k.txt_goog10-k2015.htm"));
}

TEST_F(ExportHTML, ExportHMTLFromDirectory)
{
    //	NOTE: the program name 'the_program' in the command line below is
    // ignored in the 	the test program.

    if (fs::exists("/tmp/extracts/html"))
        fs::remove_all("/tmp/extracts/html");

    std::vector<std::string> tokens{"the_program", "--log-level", "information", "--form", "10-Q,10-K", "--mode",
                                    "HTML", "--log-path", "/tmp/Extractor/test13.log",
                                    //		"--list", "./list_with_bad_file.txt",
                                    "--form-dir", SEC_DIRECTORY.string(), "--export-HTML-data", "--HTML-forms-to-dir",
                                    "/tmp/extracts/html", "--HTML-forms-from-dir", "/vol_DA/SEC/Archives"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_EQ(CountFilesInDirectoryTree("/tmp/extracts/html"), 187);
}

TEST_F(ExportHTML, VerifyNoExportOfExistingFilesWhenReplaceIsNotSpecifed)
{
    if (fs::exists("/tmp/extracts/html"))
        fs::remove_all("/tmp/extracts/html");

    std::vector<std::string> tokens{"the_program", "--log-level", "information", "--form", "10-Q,10-K", "--mode",
                                    "HTML", "--log-path", "/tmp/Extractor/test14.log",
                                    //		"--list", "./list_with_bad_file.txt",
                                    "--form-dir", SEC_DIRECTORY.string(), "-R", "--export-HTML-data",
                                    "--HTML-forms-to-dir", "/tmp/extracts/html", "--HTML-forms-from-dir",
                                    "/vol_DA/SEC/Archives"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }

    EXPECT_EQ(CountFilesInDirectoryTree("/tmp/extracts/html"), 187);

    auto x1 = CollectLastModifiedTimesForFilesInDirectoryTree("/tmp/extracts/html");

    std::this_thread::sleep_for(std::chrono::seconds{3});

    std::vector<std::string> tokens2{"the_program", "--log-level", "information", "--form", "10-Q,10-K", "--mode",
                                     "HTML", "--log-path", "/tmp/Extractor/test15.log",
                                     //		"--list", "./list_with_bad_file.txt",
                                     "--form-dir", SEC_DIRECTORY.string(), "--export-HTML-data", "--HTML-forms-to-dir",
                                     "/tmp/extracts/html", "--HTML-forms-from-dir", "/vol_DA/SEC/Archives"};

    try
    {
        ExtractorApp myApp(tokens2);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_EQ(CountFilesInDirectoryTree("/tmp/extracts/html"), 187);

    auto x2 = CollectLastModifiedTimesForFilesInDirectoryTree("/tmp/extracts/html");

    ASSERT_EQ(x1, x2);
}

TEST_F(ExportHTML, ExportHTMLUsingFileList3Async10Q)
{
    //	NOTE: the program name 'the_program' in the command line below is
    // ignored in the 	the test program.

    if (fs::exists("/tmp/extracts/html"))
        fs::remove_all("/tmp/extracts/html");

    std::vector<std::string> tokens{"the_program",
                                    "--log-level",
                                    "information",
                                    "--form",
                                    "10-Q,10-K",
                                    "--mode",
                                    "HTML",
                                    "-k",
                                    "6",
                                    "--list",
                                    "./test_directory_list.txt",
                                    "--log-path",
                                    "/tmp/Extractor/test16.log",
                                    "--export-HTML-data",
                                    "--HTML-forms-to-dir",
                                    "/tmp/extracts/html",
                                    "--HTML-forms-from-dir",
                                    "/vol_DA/SEC/Archives"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_EQ(CountFilesInDirectoryTree("/tmp/extracts/html"), 178);
}

TEST_F(ExportHTML, ExportHTMLDetectsFullDiskAndStops)
{
    //	NOTE: the program name 'the_program' in the command line below is
    // ignored in the 	the test program.

    std::vector<std::string> tokens{
        "the_program", "--log-level", "information", "--form", "10-Q,10-K", "--mode", "HTML",
        //		"-k", "6",
        //		"--list", "./test_directory_list.txt",
        "--form-dir", SEC_DIRECTORY.string(), "--log-path", "/tmp/Extractor/test17.log", "--export-HTML-data",
        "--HTML-forms-to-dir", "/tmp/ofstream_test", "--HTML-forms-from-dir", "/vol_DA/SEC/Archives"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            ASSERT_THROW(myApp.Run(), std::system_error);
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    // 	ASSERT_EQ(CountFilesInDirectoryTree("/tmp/extracts/html"), 182);
}

TEST_F(ExportHTML, AsyncExportHTMLDetectsFullDiskAndStops)
{
    //	NOTE: the program name 'the_program' in the command line below is
    // ignored in the 	the test program.

    std::vector<std::string> tokens{"the_program",
                                    "--log-level",
                                    "information",
                                    "--form",
                                    "10-Q,10-K",
                                    "--mode",
                                    "HTML",
                                    "-k",
                                    "6",
                                    "--list",
                                    "./test_directory_list.txt",
                                    "--log-path",
                                    "/tmp/Extractor/test18.log",
                                    "--export-HTML-data",
                                    "--HTML-forms-to-dir",
                                    "/tmp/ofstream_test",
                                    "--HTML-forms-from-dir",
                                    "/vol_DA/SEC/Archives"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            ASSERT_THROW(myApp.Run(), std::system_error);
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    // 	ASSERT_EQ(CountFilesInDirectoryTree("/tmp/extracts/html"), 182);
}

class UpdateSharesOutstanding : public Test
{
public:
    int entires_with_shares{0};

protected:
    void SetUp() override
    {
        pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
        pqxx::work trxn{c};

        // we want to count how many entries we have.
        // theoretically, we will update all of them.

        auto row = trxn.exec("SELECT count(*) FROM unified_extracts.sec_filing_id "
                             "WHERE shares_outstanding != -1")
                       .one_row();
        entires_with_shares = row[0].as<int>();

        trxn.exec("UPDATE unified_extracts.sec_filing_id SET shares_outstanding = -1");
        trxn.commit();
    }

public:
    int CountRows()
    {
        pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
        pqxx::work trxn{c};

        // make sure the DB is empty before we start

        auto row = trxn.exec("SELECT count(*) FROM unified_extracts.sec_filing_id "
                             "WHERE shares_outstanding != -1")
                       .one_row();
        trxn.commit();
        return row[0].as<int>();
    }
};

TEST_F(UpdateSharesOutstanding, UpdateSharesOutstandingAsyncAndSync)
{
    //	NOTE: the program name 'the_program' in the command line below is
    // ignored in the 	the test program.

    std::vector<std::string> tokens{"the_program",
                                    "--log-level",
                                    "information",
                                    "--form",
                                    "10-Q,10-K",
                                    "--mode",
                                    "HTML",
                                    "-k",
                                    "6",
                                    "--list",
                                    "./test_directory_list.txt",
                                    "--log-path",
                                    "/tmp/Extractor/test19.log",
                                    "--UpdateSharesOutstanding"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    EXPECT_NE(CountRows(), 0);
    ASSERT_EQ(CountRows(), entires_with_shares);
}

class TestBoth : public Test
{
protected:
    void SetUp() override
    {
        pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
        pqxx::work trxn{c};

        // make sure the DB is empty before we start

        trxn.exec("DELETE FROM unified_extracts.sec_filing_id");
        trxn.commit();
    }

public:
    int CountRows()
    {
        pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
        pqxx::work trxn{c};

        // make sure the DB is empty before we start

        auto row1 = trxn.exec("SELECT count(*) FROM unified_extracts.sec_bal_sheet_data").one_row();
        auto row2 = trxn.exec("SELECT count(*) FROM unified_extracts.sec_stmt_of_ops_data").one_row();
        auto row3 = trxn.exec("SELECT count(*) FROM unified_extracts.sec_cash_flows_data").one_row();
        trxn.commit();
        return row1[0].as<int>() + row2[0].as<int>() + row3[0].as<int>();
    }

    int CountMissingValues()
    {
        pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
        pqxx::work trxn{c};

        auto row1 = trxn.exec("SELECT count(*) FROM unified_extracts.sec_bal_sheet_data "
                              "WHERE label = 'Missing Value'")
                        .one_row();
        auto row2 = trxn.exec("SELECT count(*) FROM unified_extracts.sec_stmt_of_ops_data "
                              "WHERE label = 'Missing Value'")
                        .one_row();
        auto row3 = trxn.exec("SELECT count(*) FROM unified_extracts.sec_cash_flows_data "
                              "WHERE label = 'Missing Value'")
                        .one_row();
        trxn.commit();
        return row1[0].as<int>() + row2[0].as<int>() + row3[0].as<int>();
    }

    int CountFilingsHTML()
    {
        pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
        pqxx::work trxn{c};

        // make sure the DB is empty before we start

        auto row = trxn.exec("SELECT count(*) FROM unified_extracts.sec_filing_id "
                             "WHERE data_source = 'HTML'")
                       .one_row();
        trxn.commit();
        return row[0].as<int>();
    }

    int CountFilingsXBRL()
    {
        pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
        pqxx::work trxn{c};

        // make sure the DB is empty before we start

        auto row = trxn.exec("SELECT count(*) FROM unified_extracts.sec_filing_id "
                             "WHERE data_source != 'HTML'")
                       .one_row();
        trxn.commit();
        return row[0].as<int>();
    }
};

TEST_F(TestBoth, UpdateDBFromList)
{
    //	NOTE: the program name 'the_program' in the command line below is
    // ignored in the 	the test program.

    std::vector<std::string> tokens{"the_program",
                                    "--log-level",
                                    "info",
                                    "--form",
                                    "10-Q,10-K",
                                    "--mode",
                                    "BOTH",
                                    "-k",
                                    "6",
                                    "--list",
                                    "./test_directory_list.txt",
                                    "--log-path",
                                    "/tmp/Extractor/test20.log"};

    try
    {
        ExtractorApp myApp(tokens);

        decltype(auto) test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ", test_info->test_case_name(), "\n\n"));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    // there are 159 possible XBRL files but 3 of them are weirdly redundant
    // so they error out as DB duplicates.
    // also, 3 more error out because the XLS sheets can't be found

    EXPECT_EQ(CountFilingsXBRL(), 154);

    EXPECT_EQ(CountFilingsHTML(), 22);
}

/*
 * ===  FUNCTION
 * ====================================================================== Name:
 * InitLogging Description:
 * =====================================================================================
 */
void InitLogging()
{
    // spdlog::set_level(spdlog::level::info);
    //    nothing to do for now.
    //    logging::core::get()->set_filter
    //    (
    //        logging::trivial::severity >= logging::trivial::trace
    //    );
} /* -----  end of function InitLogging  ----- */

int main(int argc, char **argv)
{
    // simpler logging setup than unit test because here
    // the app class will set up required logging.

    auto my_default_logger = spdlog::stdout_color_mt("testing_logger");
    spdlog::set_default_logger(my_default_logger);

    if (fs::exists("/tmp/Extractor"))
    {
        fs::remove_all("/tmp/Extractor");
    }

    // InitLogging();

    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
