/*
 * evd-web-transport.c
 *
 * EventDance, Peer-to-peer IPC library <http://eventdance.org>
 *
 * Copyright (C) 2009/2010, Igalia S.L.
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

#include "evd-web-transport.h"
#include "evd-transport.h"

#include "evd-error.h"
#include "evd-http-connection.h"
#include "evd-peer-manager.h"
#include "evd-web-dir.h"

#include "evd-long-polling.h"

#define EVD_WEB_TRANSPORT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                            EVD_TYPE_WEB_TRANSPORT, \
                                            EvdWebTransportPrivate))

#define DEFAULT_BASE_PATH "/transport"

#define LONG_POLLING_TOKEN_NAME "lp"

/* private data */
struct _EvdWebTransportPrivate
{
  gchar *base_path;

  EvdPeerManager *peer_manager;

  EvdWebSelector *selector;

  EvdLongPolling *lp;
  gchar *lp_base_path;
};

/* properties */
enum
{
  PROP_0,
  PROP_BASE_PATH,
  PROP_SELECTOR,
  PROP_LP_SERVICE
};

static void     evd_web_transport_class_init           (EvdWebTransportClass *class);
static void     evd_web_transport_init                 (EvdWebTransport *self);

static void     evd_web_transport_transport_iface_init (EvdTransportInterface *iface);

static void     evd_web_transport_finalize             (GObject *obj);
static void     evd_web_transport_dispose              (GObject *obj);

static void     evd_web_transport_set_property         (GObject      *obj,
                                                        guint         prop_id,
                                                        const GValue *value,
                                                        GParamSpec   *pspec);
static void     evd_web_transport_get_property         (GObject    *obj,
                                                        guint       prop_id,
                                                        GValue     *value,
                                                        GParamSpec *pspec);

static void     evd_web_transport_on_receive           (EvdTransport *transport,
                                                        EvdPeer      *peer,
                                                        gpointer      user_data);

static void     evd_web_transport_on_new_peer          (EvdTransport *transport,
                                                        EvdPeer      *peer,
                                                        gpointer      user_data);

static void     evd_web_transport_on_peer_closed       (EvdTransport *transport,
                                                        EvdPeer      *peer,
                                                        gboolean      gracefully,
                                                        gpointer      user_data);

static gboolean evd_web_transport_send                 (EvdTransport  *transport,
                                                        EvdPeer       *peer,
                                                        const gchar   *buffer,
                                                        gsize          size,
                                                        GError       **error);

static gboolean evd_web_transport_peer_is_connected    (EvdTransport *transport,
                                                        EvdPeer      *peer);

static void     evd_web_transport_on_request           (EvdWebService     *self,
                                                        EvdHttpConnection *conn,
                                                        EvdHttpRequest    *request);

G_DEFINE_TYPE_WITH_CODE (EvdWebTransport, evd_web_transport, EVD_TYPE_WEB_DIR,
                         G_IMPLEMENT_INTERFACE (EVD_TYPE_TRANSPORT,
                                                evd_web_transport_transport_iface_init));

static void
evd_web_transport_class_init (EvdWebTransportClass *class)
{
  GObjectClass *obj_class = G_OBJECT_CLASS (class);
  EvdWebServiceClass *web_service_class = EVD_WEB_SERVICE_CLASS (class);

  obj_class->dispose = evd_web_transport_dispose;
  obj_class->finalize = evd_web_transport_finalize;
  obj_class->get_property = evd_web_transport_get_property;
  obj_class->set_property = evd_web_transport_set_property;

  web_service_class->request_handler = evd_web_transport_on_request;

  g_object_class_install_property (obj_class, PROP_BASE_PATH,
                                   g_param_spec_string ("base-path",
                                                        "Base path",
                                                        "URL base path the transport handles",
                                                        DEFAULT_BASE_PATH,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (obj_class, PROP_SELECTOR,
                                   g_param_spec_object ("selector",
                                                        "Transport's Web selector",
                                                        "Web selector object used by this transport to route its requests",
                                                        EVD_TYPE_WEB_SELECTOR,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (obj_class, PROP_LP_SERVICE,
                                   g_param_spec_object ("lp-service",
                                                        "Long-Polling service",
                                                        "Internal Long-Polling service used by this transport",
                                                        EVD_TYPE_LONG_POLLING,
                                                        G_PARAM_READABLE |
                                                        G_PARAM_STATIC_STRINGS));

  g_type_class_add_private (obj_class, sizeof (EvdWebTransportPrivate));
}

static void
evd_web_transport_transport_iface_init (EvdTransportInterface *iface)
{
  iface->send = evd_web_transport_send;
  iface->peer_is_connected = evd_web_transport_peer_is_connected;
}

static void
evd_web_transport_init (EvdWebTransport *self)
{
  EvdWebTransportPrivate *priv;
  const gchar *js_path;

  priv = EVD_WEB_TRANSPORT_GET_PRIVATE (self);
  self->priv = priv;

  priv->peer_manager = evd_peer_manager_get_default ();

  priv->selector = NULL;

  priv->lp = evd_long_polling_new ();
  g_signal_connect (priv->lp,
                    "receive",
                    G_CALLBACK (evd_web_transport_on_receive),
                    self);
  g_signal_connect (priv->lp,
                    "new-peer",
                    G_CALLBACK (evd_web_transport_on_new_peer),
                    self);
  g_signal_connect (priv->lp,
                    "peer-closed",
                    G_CALLBACK (evd_web_transport_on_peer_closed),
                    self);

  js_path = g_getenv ("JSLIBDIR");
  if (js_path == NULL)
    js_path = JSLIBDIR;

  evd_web_dir_set_root (EVD_WEB_DIR (self), js_path);

  evd_service_set_io_stream_type (EVD_SERVICE (self), EVD_TYPE_HTTP_CONNECTION);
}

static void
evd_web_transport_dispose (GObject *obj)
{
  G_OBJECT_CLASS (evd_web_transport_parent_class)->dispose (obj);
}

static void
evd_web_transport_finalize (GObject *obj)
{
  EvdWebTransport *self = EVD_WEB_TRANSPORT (obj);

  g_object_unref (self->priv->peer_manager);

  g_signal_handlers_disconnect_by_func (self->priv->lp,
                                        evd_web_transport_on_receive,
                                        self);
  g_object_unref (self->priv->lp);

  g_free (self->priv->lp_base_path);
  g_free (self->priv->base_path);

  G_OBJECT_CLASS (evd_web_transport_parent_class)->finalize (obj);
}

static void
evd_web_transport_set_property (GObject      *obj,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  EvdWebTransport *self;

  self = EVD_WEB_TRANSPORT (obj);

  switch (prop_id)
    {
    case PROP_BASE_PATH:
      evd_web_transport_set_base_path (self, g_value_get_string (value));
      break;

    case PROP_SELECTOR:
      evd_web_transport_set_selector (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
      break;
    }
}

static void
evd_web_transport_get_property (GObject    *obj,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  EvdWebTransport *self;

  self = EVD_WEB_TRANSPORT (obj);

  switch (prop_id)
    {
    case PROP_BASE_PATH:
      g_value_set_string (value, evd_web_transport_get_base_path (self));
      break;

    case PROP_SELECTOR:
      g_value_set_object (value, evd_web_transport_get_selector (self));
      break;

    case PROP_LP_SERVICE:
      g_value_set_object (value, self->priv->lp);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
      break;
    }
}

static void
evd_web_transport_on_receive (EvdTransport *transport,
                              EvdPeer      *peer,
                              gpointer      user_data)
{
  EvdWebTransport *self = EVD_WEB_TRANSPORT (user_data);

  EVD_TRANSPORT_GET_INTERFACE (self)->
    notify_receive (EVD_TRANSPORT (self), peer);
}

static void
evd_web_transport_on_new_peer (EvdTransport *transport,
                               EvdPeer      *peer,
                               gpointer      user_data)
{
  EvdWebTransport *self = EVD_WEB_TRANSPORT (user_data);

  EVD_TRANSPORT_GET_INTERFACE (self)->
    notify_new_peer (EVD_TRANSPORT (self), peer);
}

static void
evd_web_transport_on_peer_closed (EvdTransport *transport,
                                  EvdPeer      *peer,
                                  gboolean      gracefully,
                                  gpointer      user_data)
{
  EvdWebTransport *self = EVD_WEB_TRANSPORT (user_data);

  EVD_TRANSPORT_GET_INTERFACE (self)->
    notify_peer_closed (EVD_TRANSPORT (self), peer, gracefully);
}

static gboolean
evd_web_transport_validate_peer_transport (EvdWebTransport  *self,
                                           EvdTransport     *peer_transport,
                                           GError          **error)
{
  if (peer_transport != EVD_TRANSPORT (self->priv->lp))
    {
      g_set_error_literal (error,
                           EVD_ERROR,
                           EVD_ERROR_INVALID_DATA,
                           "Invalid peer transport");

      return FALSE;
    }

  return TRUE;
}

static gboolean
evd_web_transport_send (EvdTransport  *transport,
                        EvdPeer       *peer,
                        const gchar   *buffer,
                        gsize          size,
                        GError       **error)
{
  EvdWebTransport *self = EVD_WEB_TRANSPORT (transport);
  EvdTransport *_transport;
  gssize result = 0;

  g_object_get (peer, "transport", &_transport, NULL);

  if (! evd_web_transport_validate_peer_transport (self, _transport, error)
      || ! evd_transport_send (_transport,
                               peer,
                               buffer,
                               size,
                               error))
    {
      result = FALSE;
    }
  else
    {
      result = TRUE;
    }

  g_object_unref (_transport);

  return result;
}

static gboolean
evd_web_transport_peer_is_connected (EvdTransport *transport,
                                     EvdPeer      *peer)
{
  EvdWebTransport *self = EVD_WEB_TRANSPORT (transport);
  EvdTransport *_transport;
  gboolean result = FALSE;

  g_object_get (peer, "transport", &_transport, NULL);

  if (! evd_web_transport_validate_peer_transport (self, transport, NULL))
    result = FALSE;
  else
    result = evd_transport_peer_is_connected (_transport, peer);

  g_object_unref (_transport);

  return result;
}

static void
evd_web_transport_associate_services (EvdWebTransport *self)
{
  if (self->priv->selector == NULL)
    return;

  evd_web_selector_add_service (self->priv->selector,
                                NULL,
                                self->priv->base_path,
                                EVD_SERVICE (self),
                                NULL);
}

static void
evd_web_transport_unassociate_services (EvdWebTransport *self)
{
  if (self->priv->selector == NULL)
    return;

  evd_web_selector_remove_service (self->priv->selector,
                                   NULL,
                                   self->priv->base_path,
                                   EVD_SERVICE (self));
}

static void
evd_web_transport_on_request (EvdWebService     *web_service,
                              EvdHttpConnection *conn,
                              EvdHttpRequest    *request)
{
  EvdWebTransport *self = EVD_WEB_TRANSPORT (web_service);
  SoupURI *uri;

  uri = evd_http_request_get_uri (request);

  if (g_strstr_len (uri->path, -1, self->priv->lp_base_path) == uri->path)
    {
      evd_web_service_add_connection_with_request (EVD_WEB_SERVICE (self->priv->lp),
                                                   conn,
                                                   request,
                                                   NULL);
    }
  else
    {
      EVD_WEB_SERVICE_CLASS (evd_web_transport_parent_class)->
        request_handler (web_service,
                         conn,
                         request);
    }
}

/* public methods */

EvdWebTransport *
evd_web_transport_new (void)
{
  EvdWebTransport *self;

  self = g_object_new (EVD_TYPE_WEB_TRANSPORT, NULL);

  return self;
}

void
evd_web_transport_set_selector (EvdWebTransport *self,
                                EvdWebSelector  *selector)
{
  g_return_if_fail (EVD_IS_WEB_TRANSPORT (self));
  g_return_if_fail (EVD_IS_WEB_SELECTOR (selector));

  if (self->priv->selector != NULL)
    {
      evd_web_transport_unassociate_services (self);
      g_object_unref (self->priv->selector);
    }

  self->priv->selector = selector;

  evd_web_transport_associate_services (self);
  g_object_ref (selector);
}

EvdWebSelector *
evd_web_transport_get_selector (EvdWebTransport *self)
{
  g_return_val_if_fail (EVD_IS_WEB_TRANSPORT (self), NULL);

  return self->priv->selector;
}

void
evd_web_transport_set_base_path (EvdWebTransport *self,
                                 const gchar     *base_path)
{
  g_return_if_fail (EVD_IS_WEB_TRANSPORT (self));
  g_return_if_fail (base_path != NULL);

  if (self->priv->base_path != NULL)
    {
      evd_web_transport_unassociate_services (self);
      g_free (self->priv->base_path);
      g_free (self->priv->lp_base_path);
    }

  self->priv->base_path = g_strdup (base_path);
  evd_web_transport_associate_services (self);

  self->priv->lp_base_path = g_strdup_printf ("%s/%s",
                                              self->priv->base_path,
                                              LONG_POLLING_TOKEN_NAME);

  evd_web_dir_set_alias (EVD_WEB_DIR (self), base_path);
}

const gchar *
evd_web_transport_get_base_path (EvdWebTransport *self)
{
  g_return_val_if_fail (EVD_IS_WEB_TRANSPORT (self), NULL);

  return self->priv->base_path;
}
