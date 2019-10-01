/*******************************************************************************
*   (c) 2018 ZondaX GmbH
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include "gtest/gtest.h"
#include <string>
#include <lib/json/json_parser.h>
#include <lib/json/tx_display.h>
#include "util/common.h"

namespace {
    TEST(TxParse, Tx_Traverse) {
        auto transaction = R"({"keyA":"123456", "keyB":"abcdefg", "keyC":""})";

        parsed_json_t parsed_json;
        auto err = parse_tx(&parsed_json, transaction, 100);
        ASSERT_STREQ(nullptr, err);

        tx_display_index_root();

        char key[100];
        char val[100];
        int16_t found;

        // Try second key - first chunk
        INIT_QUERY_CONTEXT(key, sizeof(key), val, sizeof(val), 0, 4);
        parser_tx_obj.tx_ctx.query.item_index = 1;
        found = tx_traverse(0);
        EXPECT_EQ(1, found) << "Item not found";
        EXPECT_EQ_STR(key, "keyB", "Invalid key");
        EXPECT_EQ_STR(val, "abcdefg", "Invalid value");

        // Try second key - Second chunk
        INIT_QUERY_CONTEXT(key, sizeof(key), val, sizeof(val), 1, 4);
        parser_tx_obj.tx_ctx.query.item_index = 1;
        found = tx_traverse(0);
        EXPECT_EQ(TX_TOKEN_NOT_FOUND, found) << "Item not found";

        // Find first key
        INIT_QUERY_CONTEXT(key, sizeof(key), val, sizeof(val), 0, 4);
        parser_tx_obj.tx_ctx.query.item_index = 0;
        found = tx_traverse(0);
        EXPECT_EQ(1, found) << "Item not found";
        EXPECT_EQ_STR(key, "keyA", "Invalid key");
        EXPECT_EQ_STR(val, "123456", "Invalid value");

        // Try the same again
        INIT_QUERY_CONTEXT(key, sizeof(key), val, sizeof(val), 0, 4);
        parser_tx_obj.tx_ctx.query.item_index = 0;
        found = tx_traverse(0);
        EXPECT_EQ(1, found) << "Item not found";
        EXPECT_EQ_STR(key, "keyA", "Invalid key");
        EXPECT_EQ_STR(val, "123456", "Invalid value");

        // Try last key
        INIT_QUERY_CONTEXT(key, sizeof(key), val, sizeof(val), 0, 4);
        parser_tx_obj.tx_ctx.query.item_index = 2;
        found = tx_traverse(0);
        EXPECT_EQ(1, found) << "Item not found";
        EXPECT_EQ_STR(key, "keyC", "Invalid key");
        EXPECT_EQ_STR(val, "", "Invalid value");
    }

    TEST(TxParse, OutOfBoundsSmall) {
        auto transaction = R"({"keyA":"123456", "keyB":"abcdefg"})";

        parsed_json_t parsed_json;
        auto err = parse_tx(&parsed_json, transaction, 100);
        ASSERT_STREQ(nullptr, err);

        char key[100];
        char val[100];
        int16_t found;

        INIT_QUERY_CONTEXT(key, sizeof(key), val, sizeof(val), 5, 4);
        found = tx_traverse(0);
        EXPECT_EQ(TX_TOKEN_NOT_FOUND, found) << "Item not found";

        // We should find it.. but later tx_display should fail
        INIT_QUERY_CONTEXT(key, sizeof(key), val, sizeof(val), 0, 4);
        found = tx_traverse(0);
        EXPECT_EQ(1, found) << "Item not found";
    }

    TEST(TxParse, Count_Minimal) {
        auto transaction = R"({"account_number":"0"})";

        parsed_json_t parsed_json;
        auto err = parse_tx(&parsed_json, transaction, 100);
        ASSERT_STREQ(nullptr, err);

        auto num_pages = tx_display_numItems();

        EXPECT_EQ(0, num_pages) << "Wrong number of pages";
    }

    TEST(TxParse, Tx_Page_Count) {

        auto transaction = R"({"account_number":"0","chain_id":"test-chain-1","fee":{"amount":[{"amount":"5","denom":"photon"}],"gas":"10000"},"memo":"testmemo","msgs":[{"inputs":[{"address":"cosmosaccaddr1d9h8qat5e4ehc5","coins":[{"amount":"10","denom":"atom"}]}],"outputs":[{"address":"cosmosaccaddr1da6hgur4wse3jx32","coins":[{"amount":"10","denom":"atom"}]}]}],"sequence":"1"})";

        parsed_json_t parsed_json;
        auto err = parse_tx(&parsed_json, transaction, 100);
        ASSERT_STREQ(nullptr, err);

        auto num_pages = tx_display_numItems();
        EXPECT_EQ(10, num_pages) << "Wrong number of pages";
    }

    TEST(TxParse, Page_Count_MultipleMsgs) {

        auto transaction =
            R"({"account_number":"0","chain_id":"test-chain-1","fee":{"amount":[{"amount":"5","denom":"photon"}],"gas":"10000"},"memo":"testmemo","msgs":[{"inputs":[{"address":"cosmosaccaddr1d9h8qat5e4ehc5","coins":[{"amount":"10","denom":"atom"}]}],"outputs":[{"address":"cosmosaccaddr1da6hgur4wse3jx32","coins":[{"amount":"10","denom":"atom"}]}]},{"inputs":[{"address":"cosmosaccaddr1d9h8qat5e4ehc5","coins":[{"amount":"10","denom":"atom"}]}],"outputs":[{"address":"cosmosaccaddr1da6hgur4wse3jx32","coins":[{"amount":"10","denom":"atom"}]}]},{"inputs":[{"address":"cosmosaccaddr1d9h8qat5e4ehc5","coins":[{"amount":"10","denom":"atom"}]}],"outputs":[{"address":"cosmosaccaddr1da6hgur4wse3jx32","coins":[{"amount":"10","denom":"atom"}]}]},{"inputs":[{"address":"cosmosaccaddr1d9h8qat5e4ehc5","coins":[{"amount":"10","denom":"atom"}]}],"outputs":[{"address":"cosmosaccaddr1da6hgur4wse3jx32","coins":[{"amount":"10","denom":"atom"}]}]}],"sequence":"1"})";

        parsed_json_t parsed_json;
        auto err = parse_tx(&parsed_json, transaction, 100);
        ASSERT_STREQ(nullptr, err);

        EXPECT_EQ(22, tx_display_numItems()) << "Wrong number of items";
    }
}
