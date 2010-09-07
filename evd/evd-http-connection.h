/*
 * evd-http-connection.h
 *
 * EventDance - An event distribution framework (http://eventdance.org)
 *
 * Copyright (C) 2009/2010, Igalia S.L.
 *
 * Authors:
 *   Eduardo Lima Mitev <elima@igalia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License at http://www.gnu.org/licenses/lgpl-3.0.txt
 * for more details.
 */

#ifndef __EVD_HTTP_CONNECTION_H__
#define __EVD_HTTP_CONNECTION_H__

#include <glib-object.h>
#include <libsoup/soup-headers.h>

#include <evd-connection.h>

G_BEGIN_DECLS

typedef struct _EvdHttpConnection EvdHttpConnection;
typedef struct _EvdHttpConnectionClass EvdHttpConnectionClass;
typedef struct _EvdHttpConnectionPrivate EvdHttpConnectionPrivate;

struct _EvdHttpConnection
{
  EvdConnection parent;

  EvdHttpConnectionPrivate *priv;
};

struct _EvdHttpConnectionClass
{
  EvdConnectionClass parent_class;
};

#define EVD_TYPE_HTTP_CONNECTION           (evd_http_connection_get_type ())
#define EVD_HTTP_CONNECTION(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), EVD_TYPE_HTTP_CONNECTION, EvdHttpConnection))
#define EVD_HTTP_CONNECTION_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST ((obj), EVD_TYPE_HTTP_CONNECTION, EvdHttpConnectionClass))
#define EVD_IS_HTTP_CONNECTION(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EVD_TYPE_HTTP_CONNECTION))
#define EVD_IS_HTTP_CONNECTION_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE ((obj), EVD_TYPE_HTTP_CONNECTION))
#define EVD_HTTP_CONNECTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EVD_TYPE_HTTP_CONNECTION, EvdHttpConnectionClass))


GType               evd_http_connection_get_type                     (void) G_GNUC_CONST;

EvdHttpConnection  *evd_http_connection_new                          (EvdSocket *socket);

void                evd_http_connection_read_response_headers_async  (EvdHttpConnection   *self,
                                                                      GCancellable        *cancellable,
                                                                      GAsyncReadyCallback callback,
                                                                      gpointer            user_data);
SoupMessageHeaders *evd_http_connection_read_response_headers_finish (EvdHttpConnection   *self,
                                                                      GAsyncResult        *result,
                                                                      SoupHTTPVersion     *version,
                                                                      guint               *status_code,
                                                                      gchar              **reason_phrase,
                                                                      GError             **error);

void                evd_http_connection_read_request_headers_async   (EvdHttpConnection   *self,
                                                                      GCancellable        *cancellable,
                                                                      GAsyncReadyCallback callback,
                                                                      gpointer            user_data);

SoupMessageHeaders *evd_http_connection_read_request_headers_finish  (EvdHttpConnection   *self,
                                                                      GAsyncResult        *result,
                                                                      SoupHTTPVersion     *version,
                                                                      gchar              **method,
                                                                      gchar              **path,
                                                                      GError             **error);

gboolean            evd_http_connection_write_response_headers       (EvdHttpConnection   *self,
                                                                      SoupHTTPVersion      version,
                                                                      guint                status_code,
                                                                      gchar               *reason_phrase,
                                                                      SoupMessageHeaders  *headers,
                                                                      GCancellable        *cancellable,
                                                                      GError             **error);
gboolean            evd_http_connection_write_content                (EvdHttpConnection  *self,
                                                                      const gchar        *buffer,
                                                                      gsize               size,
                                                                      GCancellable       *cancellable,
                                                                      GError            **error);

void                evd_http_connection_read_all_content_async       (EvdHttpConnection   *self,
                                                                      GCancellable        *cancellable,
                                                                      GAsyncReadyCallback  callback,
                                                                      gpointer             user_data);
gchar *             evd_http_connection_read_all_content_finish      (EvdHttpConnection  *self,
                                                                      GAsyncResult       *result,
                                                                      gssize             *size,
                                                                      GError            **error);

gboolean            evd_http_connection_unread_request_headers       (EvdHttpConnection   *self,
                                                                      SoupHTTPVersion      version,
                                                                      const gchar         *method,
                                                                      const gchar         *path,
                                                                      SoupMessageHeaders  *headers,
                                                                      GCancellable        *cancellable,
                                                                      GError             **error);

gboolean            evd_http_connection_respond                      (EvdHttpConnection   *self,
                                                                      SoupHTTPVersion      ver,
                                                                      guint                status_code,
                                                                      gchar               *reason_phrase,
                                                                      SoupMessageHeaders  *headers,
                                                                      const gchar         *content,
                                                                      gsize                size,
                                                                      gboolean             close_after,
                                                                      GCancellable        *cancellable,
                                                                      GError             **error);

G_END_DECLS

#endif /* __EVD_HTTP_CONNECTION_H__ */