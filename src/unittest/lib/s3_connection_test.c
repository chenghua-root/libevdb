#include "third/ctest/ctest.h"
#include "lib/s3_connection.h"
#include "lib/s3_error.h"

TEST(test_connection, connection_init) {
    S3Connection *c = s3_connection_construct();
    ASSERT_NE(NULL, c);

    ASSERT_EQ(S3_OK, s3_connection_init(c, NULL, -1));

    s3_connection_destruct(c);
}
