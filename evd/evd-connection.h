/*
 * evd-connection.h
 *
 * EventDance, Peer-to-peer IPC library <http://eventdance.org>
 *
 * Copyright (C) 2009-2013, Igalia S.L.
 *
 * Authors:
 *   Eduardo Lima Mitev <elima@igalia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 3, or (at your option) any later version as published by
 * the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License at http://www.gnu.org/licenses/lgpl-3.0.txt
 * for more details.
 */

#ifndef __EVD_CONNECTION_H__
#define __EVD_CONNECTION_H__

#if !defined (__EVD_H_INSIDE__) && !defined (EVD_COMPILATION)
#error "Only <evd.h> can be included directly."
#endif

#include <glib-object.h>
#include <gio/gio.h>

#include "evd-io-stream.h"
#include "evd-socket.h"
#include "evd-tls-session.h"
#include "evd-io-stream-group.h"

G_BEGIN_DECLS

typedef struct _EvdConnection EvdConnection;
typedef struct _EvdConnectionClass EvdConnectionClass;
typedef struct _EvdConnectionPrivate EvdConnectionPrivate;

struct _EvdConnection
{
  EvdIoStream parent;

  EvdConnectionPrivate *priv;
};

struct _EvdConnectionClass
{
  EvdIoStreamClass parent_class;

  /* signal prototypes */
  void (* close)         (EvdConnection *self);
  void (* write)         (EvdConnection *self);

  /* padding for future expansion */
  void (* _padding_0_) (void);
  void (* _padding_1_) (void);
  void (* _padding_2_) (void);
  void (* _padding_3_) (void);
  void (* _padding_4_) (void);
  void (* _padding_5_) (void);
  void (* _padding_6_) (void);
  void (* _padding_7_) (void);
};

#define EVD_TYPE_CONNECTION           (evd_connection_get_type ())
#define EVD_CONNECTION(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), EVD_TYPE_CONNECTION, EvdConnection))
#define EVD_CONNECTION_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST ((obj), EVD_TYPE_CONNECTION, EvdConnectionClass))
#define EVD_IS_CONNECTION(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EVD_TYPE_CONNECTION))
#define EVD_IS_CONNECTION_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE ((obj), EVD_TYPE_CONNECTION))
#define EVD_CONNECTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EVD_TYPE_CONNECTION, EvdConnectionClass))


GType              evd_connection_get_type             (void) G_GNUC_CONST;

EvdConnection     *evd_connection_new                  (EvdSocket *socket);

void               evd_connection_set_socket           (EvdConnection *self,
                                                        EvdSocket     *socket);
EvdSocket         *evd_connection_get_socket           (EvdConnection *self);

EvdTlsSession     *evd_connection_get_tls_session      (EvdConnection *self);

void               evd_connection_starttls             (EvdConnection       *self,
                                                        EvdTlsMode           mode,
                                                        GCancellable        *cancellable,
                                                        GAsyncReadyCallback  callback,
                                                        gpointer             user_data);
gboolean           evd_connection_starttls_finish      (EvdConnection  *self,
                                                        GAsyncResult   *result,
                                                        GError        **error);

gboolean           evd_connection_get_tls_active       (EvdConnection *self);

gsize              evd_connection_get_max_readable     (EvdConnection *self);
gsize              evd_connection_get_max_writable     (EvdConnection *self);

gboolean           evd_connection_is_connected         (EvdConnection *self);

gint               evd_connection_get_priority         (EvdConnection *self);

void               evd_connection_lock_close           (EvdConnection *self);
void               evd_connection_unlock_close         (EvdConnection *self);

void               evd_connection_flush_and_shutdown   (EvdConnection  *self,
                                                        GCancellable   *cancellable);

gchar *            evd_connection_get_remote_address_as_string (EvdConnection  *self,
                                                                GError        **error);

G_END_DECLS

#endif /* __EVD_CONNECTION_H__ */
