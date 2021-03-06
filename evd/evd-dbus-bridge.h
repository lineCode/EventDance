/*
 * evd-dbus-bridge.h
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

#ifndef __EVD_DBUS_BRIDGE_H__
#define __EVD_DBUS_BRIDGE_H__

#if !defined (__EVD_H_INSIDE__) && !defined (EVD_COMPILATION)
#error "Only <evd.h> can be included directly."
#endif

#include <glib-object.h>

#include "evd-ipc-mechanism.h"

G_BEGIN_DECLS

typedef struct _EvdDBusBridge EvdDBusBridge;
typedef struct _EvdDBusBridgeClass EvdDBusBridgeClass;
typedef struct _EvdDBusBridgePrivate EvdDBusBridgePrivate;

struct _EvdDBusBridge
{
  EvdIpcMechanism parent;

  EvdDBusBridgePrivate *priv;
};

struct _EvdDBusBridgeClass
{
  EvdIpcMechanismClass parent_class;
};

#define EVD_TYPE_DBUS_BRIDGE           (evd_dbus_bridge_get_type ())
#define EVD_DBUS_BRIDGE(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), EVD_TYPE_DBUS_BRIDGE, EvdDBusBridge))
#define EVD_DBUS_BRIDGE_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST ((obj), EVD_TYPE_DBUS_BRIDGE, EvdDBusBridgeClass))
#define EVD_IS_DBUS_BRIDGE(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EVD_TYPE_DBUS_BRIDGE))
#define EVD_IS_DBUS_BRIDGE_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE ((obj), EVD_TYPE_DBUS_BRIDGE))
#define EVD_DBUS_BRIDGE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EVD_TYPE_DBUS_BRIDGE, EvdDBusBridgeClass))


GType                   evd_dbus_bridge_get_type                (void) G_GNUC_CONST;

EvdDBusBridge *         evd_dbus_bridge_new                     (void);

#ifdef ENABLE_TESTS
/* only for testing purposes, DO NOT use in your programs */

typedef void (* EvdDBusBridgeSendMsgCb) (EvdDBusBridge *self,
                                         GObject       *object,
                                         const gchar   *json,
                                         gpointer       user_data);

void                    evd_dbus_bridge_process_msg             (EvdDBusBridge *self,
                                                                 GObject       *object,
                                                                 const gchar   *msg,
                                                                 gsize          length);

void                    evd_dbus_bridge_set_send_msg_callback   (EvdDBusBridge          *self,
                                                                 EvdDBusBridgeSendMsgCb  callback,
                                                                 gpointer                user_data);

void                    evd_dbus_bridge_track_object            (EvdDBusBridge *self,
                                                                 GObject       *object);

#endif /* ENABLE_TESTS */

G_END_DECLS

#endif /* __EVD_DBUS_BRIDGE_H__ */
