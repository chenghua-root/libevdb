#include "third/ctest/ctest.h"
#include "lib/s3_connection.h"

TEST(test_connection, connection_init) {
    S3Connection *conn = s3_connection_construct();
    ASSERT_NE(NULL, conn);

    s3_connection_desconstruct(conn);
    s3_connection_desconstruct(conn);
}
