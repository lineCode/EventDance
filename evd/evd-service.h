/*
 * evd-service.h
 *
 * EventDance project - An event distribution framework (http://eventdance.org)
 *
 * Copyright (C) 2009, Igalia S.L.
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
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef __EVD_SERVICE_H__
#define __EVD_SERVICE_H__

#include <glib-object.h>
#include <gio/gio.h>

#include <evd-io-stream-group.h>
#include <evd-connection.h>
#include <evd-tls-credentials.h>

G_BEGIN_DECLS

typedef struct _EvdService EvdService;
typedef struct _EvdServiceClass EvdServiceClass;
typedef struct _EvdServicePrivate EvdServicePrivate;

struct _EvdService
{
  EvdIoStreamGroup parent;

  EvdServicePrivate *priv;
};

struct _EvdServiceClass
{
  EvdIoStreamGroupClass parent_class;

  /* virtual methods */
  void     (* accept_connection) (EvdService     *self,
                                  EvdConnection  *conn);
  void (* connection_closed)   (EvdService     *self,
                                EvdConnection  *conn);

  /* signal prototypes */
  guint (* validate_connection)  (EvdService    *self,
                                  EvdConnection *socket,
                                  gpointer       user_data);

  void  (* close_signal)           (EvdService *self);
};

typedef enum
{
  EVD_SERVICE_VALIDATE_ACCEPT,
  EVD_SERVICE_VALIDATE_REJECT,
  EVD_SERVICE_VALIDATE_PENDING
} EvdServiceValidate;

#define EVD_TYPE_SERVICE           (evd_service_get_type ())
#define EVD_SERVICE(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), EVD_TYPE_SERVICE, EvdService))
#define EVD_SERVICE_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST ((obj), EVD_TYPE_SERVICE, EvdServiceClass))
#define EVD_IS_SERVICE(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EVD_TYPE_SERVICE))
#define EVD_IS_SERVICE_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE ((obj), EVD_TYPE_SERVICE))
#define EVD_SERVICE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EVD_TYPE_SERVICE, EvdServiceClass))


GType              evd_service_get_type            (void) G_GNUC_CONST;

EvdService        *evd_service_new                 (void);

void               evd_service_set_tls_autostart   (EvdService *self,
                                                    gboolean    autostart);
gboolean           evd_service_get_tls_autostart   (EvdService *self);

void               evd_service_set_tls_credentials (EvdService        *self,
                                                    EvdTlsCredentials *credentials);
EvdTlsCredentials *evd_service_get_tls_credentials (EvdService *self);

void               evd_service_set_io_stream_type  (EvdService *self,
                                                    GType       io_stream_type);

void               evd_service_add_listener        (EvdService  *self,
                                                    EvdSocket   *socket);

gboolean           evd_service_remove_listener     (EvdService *self,
                                                    EvdSocket  *socket);

void               evd_service_listen_async        (EvdService          *self,
                                                    const gchar         *address,
                                                    GCancellable        *cancellable,
                                                    GAsyncReadyCallback  callback,
                                                    gpointer             user_data);
gboolean           evd_service_listen_finish       (EvdService    *self,
                                                    GAsyncResult  *result,
                                                    GError       **error);

G_END_DECLS

#endif /* __EVD_SERVICE_H__ */
