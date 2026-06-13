#include <check.h>
#include <stdlib.h>
#include <string.h>

/* Include the server header or declare the relevant structure/function */
#include "source/server.c"

START_TEST(test_username_buffer_overflow)
{
    /* Invariant: Setting a username must never write beyond the bounds of the
       username buffer, regardless of input length. */
    const char *payloads[] = {
        /* Exact exploit: username far exceeding any reasonable buffer size */
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
        /* Boundary: exactly at typical buffer size (e.g., 64 bytes including null) */
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
        /* Valid short input */
        "alice",
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);

    for (int i = 0; i < num_payloads; i++) {
        /* Allocate a server instance with a canary after the structure to detect overflow */
        size_t alloc_size = sizeof(struct server) + 64;
        char *block = calloc(1, alloc_size);
        ck_assert_ptr_nonnull(block);

        /* Fill canary region with known pattern */
        memset(block + sizeof(struct server), 0xDE, 64);

        struct server *srv = (struct server *)block;

        /* Call the production function that sets the username */
        server_set_username(srv, payloads[i]);

        /* The username stored must be properly null-terminated within bounds */
        size_t stored_len = strlen(srv->username);
        ck_assert_msg(stored_len < sizeof(srv->username),
                      "Username overflows buffer: stored %zu bytes, buffer is %zu",
                      stored_len, sizeof(srv->username));

        /* Canary must be intact - no write past the structure */
        char expected_canary[64];
        memset(expected_canary, 0xDE, 64);
        ck_assert_msg(memcmp(block + sizeof(struct server), expected_canary, 64) == 0,
                      "Memory corruption detected beyond server struct with payload %d", i);

        free(block);
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_username_buffer_overflow);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}