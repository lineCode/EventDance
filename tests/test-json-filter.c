/*
 * test-json-filter.c
 *
 * EventDance, Peer-to-peer IPC library <http://eventdance.org>
 *
 * Copyright (C) 2009-2014, Igalia S.L.
 *
 * Authors:
 *   Eduardo Lima Mitev <elima@igalia.com>
 */

#include <glib.h>
#include <gio/gio.h>
#include <string.h>

#include "evd-json-filter.h"

static const gchar *evd_json_filter_chunks[] =
{
  "   [\"hell",
  "o world!\", 1, 4, fal",
  "se,    456, 4,   ",
  "null]      {\"foo\":1234} "
};

static const gchar *evd_json_filter_packets[] =
{
  "[\"hello world!\", 1, 4, false,    456, 4,   null]",
  "{\"foo\":1234} "
};

typedef struct
{
  EvdJsonFilter *filter;
  gint packet_index;
} EvdJsonFilterFixture;

void
evd_json_filter_fixture_setup (EvdJsonFilterFixture *f,
                               gconstpointer         test_data)
{
  f->filter = evd_json_filter_new ();

  f->packet_index = 0;
}

void
evd_json_filter_fixture_teardown (EvdJsonFilterFixture *f,
                                  gconstpointer         test_data)
{
  g_object_unref (f->filter);
}

static void
evd_json_filter_test_basic (EvdJsonFilterFixture *f,
                            gconstpointer         test_data)
{
  gint i;
  GError *error = NULL;
  const gchar *wrong[] =
    {
      "null",
      "true",
      "false",
      "1",
      "\"hello world!\"",
      "{]",
      "[}",
      "}}",
      "]]",
      "{foo: 123}",
      "{\"foo\":]",
      "{:\"bar\"]",
      "[\"bar\",]"
    };

  const gchar *good[] =
    {
      "{}",
      "[]",
      "  {  }  [  ] ",
      "{\"foo\":123}",
      "[null,true,false]",
      "[1, 0.01, 3.12e5, -666.99E+12, -0.23e-5]",
      "[\"hello world!\", \"foo (\\\"bar') \"]",
      "{\"obj\":{\"null\": true},\"arr\":[false]}"
    };

  /* wrong */
  for (i=0; i<sizeof (wrong) / sizeof (gchar *); i++)
    {
      g_assert (! evd_json_filter_feed_len (f->filter,
                                            wrong[i],
                                            strlen (wrong[i]),
                                            &error));

      g_assert_error (error,
                      G_IO_ERROR,
                      G_IO_ERROR_INVALID_DATA);

      g_error_free (error);
      error = NULL;
    }

  /* good */
  for (i=0; i<sizeof (good) / sizeof (gchar *); i++)
    {
      g_assert (evd_json_filter_feed (f->filter, good[i], &error));
      g_assert_no_error (error);
    }
}

static void
evd_json_filter_test_chunked_on_packet (EvdJsonFilter *filter,
                                        const gchar   *buffer,
                                        gsize          size,
                                        gpointer       user_data)
{
  EvdJsonFilterFixture *f = (EvdJsonFilterFixture *) user_data;

  g_assert (EVD_IS_JSON_FILTER (filter));

  g_assert_cmpint (g_strcmp0 (buffer, evd_json_filter_packets[f->packet_index]),
                   ==,
                   0);

  f->packet_index++;
}

static void
evd_json_filter_test_chunked (EvdJsonFilterFixture *f,
                              gconstpointer         test_data)
{
  gint i;
  GError *error = NULL;

  evd_json_filter_set_packet_handler (f->filter,
          (EvdJsonFilterOnPacketHandler) evd_json_filter_test_chunked_on_packet,
          (gpointer) f,
          NULL);

  for (i=0; i<sizeof (evd_json_filter_chunks) / sizeof (gchar *); i++)
    {
      g_assert (evd_json_filter_feed_len (f->filter,
                                          evd_json_filter_chunks[i],
                                          strlen (evd_json_filter_chunks[i]),
                                          &error));
      g_assert_no_error (error);
    }
}

gint
main (gint argc, gchar *argv[])
{
#ifndef GLIB_VERSION_2_36
  g_type_init ();
#endif

  g_test_init (&argc, &argv, NULL);

  g_test_add ("/evd/json/filter/basic",
              EvdJsonFilterFixture,
              NULL,
              evd_json_filter_fixture_setup,
              evd_json_filter_test_basic,
              evd_json_filter_fixture_teardown);

  g_test_add ("/evd/json/filter/chunked",
              EvdJsonFilterFixture,
              NULL,
              evd_json_filter_fixture_setup,
              evd_json_filter_test_chunked,
              evd_json_filter_fixture_teardown);

  return g_test_run ();
}
