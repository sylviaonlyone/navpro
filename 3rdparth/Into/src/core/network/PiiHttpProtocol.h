/* This file is part of Into.
 * Copyright (C) 2003-2011 Intopii.
 * All rights reserved.
 *
 * IMPORTANT LICENSING INFORMATION
 *
 * Into is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License version 3
 * as published by the Free Software Foundation.
 *
 * Into is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef _PIIHTTPPROTOCOL_H
#define _PIIHTTPPROTOCOL_H

#include "PiiNetworkProtocol.h"
#include <QPair>
#include <QMutex>
#include <QTime>
#include <PiiProgressController.h>

class PiiHttpDevice;

/**
 * An implementation of the HTTP protocol.
 *
 * The role of @p %PiiHttpProtocol is to map server URIs into <em>URI
 * handlers</em>. When a request comes in, the server looks at the
 * request URI and sequentially matches its beginning to registered
 * handlers. The handler with the most specific match will be given
 * the task to encode the request body and to reply to the client. @p
 * %PiiHttpProtocol uses PiiHttpDevice as the communication channel.
 *
 * All functions in this class are thread-safe.
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiHttpProtocol : public PiiNetworkProtocol
{
public:
  /**
   * Known HTTP status codes. This is not a complete list of
   * application-specific status codes, but covers most typical uses.
   */
  enum Status
    {
      ContinueStatus = 100,
      SwitchingProtocolsStatus = 101,
      ProcessingStatus = 102,
      OkStatus = 200,
      CreatedStatus = 201,
      AcceptedStatus = 202,
      NonAuthoritativeInformationStatus = 203,
      NoContentStatus = 204,
      ResetContentStatus = 205,
      PartialContentStatus = 206,
      MultiStatus = 207,
      ImUsedStatus = 226,
      MultipleChoicesStatus = 300,
      MovedPermanentlyStatus = 301,
      FoundStatus = 302,
      SeeOtherStatus = 303,
      NotModifiedStatus = 304,
      UseProxyStatus = 305,
      ReservedStatus = 306,
      TemporaryRedirectStatus = 307,
      BadRequestStatus = 400,
      UnauthorizedStatus = 401,
      PaymentRequiredStatus = 402,
      ForbiddenStatus = 403,
      NotFoundStatus = 404,
      MethodNotAllowedStatus = 405,
      NotAcceptableStatus = 406,
      ProxyAuthenticationRequiredStatus = 407,
      RequestTimeoutStatus = 408,
      ConflictStatus = 409,
      GoneStatus = 410,
      LengthRequiredStatus = 411,
      PreconditionFailedStatus = 412,
      RequestEntityTooLargeStatus = 413,
      RequestUriTooLongStatus = 414,
      UnsupportedMediaTypeStatus = 415,
      RequestedRangeNotSatisfiableStatus = 416,
      ExpectationFailedStatus = 417,
      UnprocessableEntityStatus = 422,
      LockedStatus = 423,
      FailedDependencyStatus = 424,
      UpgradeRequiredStatus = 426,
      InternalServerErrorStatus = 500,
      NotImplementedStatus = 501,
      BadGatewayStatus = 502,
      ServiceUnavailableStatus = 503,
      GatewayTimeoutStatus = 504,
      HttpVersionNotSupportedStatus = 505,
      VariantAlsoNegotiatesStatus = 506,
      InsufficientStorageStatus = 507,
      NotExtendedStatus = 510
    };

  /**
   * Limits the time a URI handler can run.
   */
  class PII_NETWORK_EXPORT TimeLimiter : public PiiProgressController
  {
  public:
    bool canContinue(double progressPercentage = NAN);

    void setMaxTime(int maxTime);
    int maxTime() const;
    
  private:
    friend class PiiHttpProtocol;
    TimeLimiter() {}
    TimeLimiter(PiiProgressController* controller, int maxTime);

    PiiProgressController* _pController;
    int _iMaxTime;
    QTime _time;
  };

  /**
   * An interface for objects that handle requests to specified URIs.
   */
  class PII_NETWORK_EXPORT UriHandler
  {
  public:
    virtual ~UriHandler();
    
    /**
     * Handles a request. This function must be thread-safe.
     *
     * @param uri the URI the handler was registered at. Use the
     * @ref PiiHttpDevice::requestUri() function to fetch the full
     * request URI.
     *
     * @param dev the communication device. @p %PiiHttpProtocol has
     * already fetched request headers, and the device is positioned
     * at the beginning of request data.
     *
     * @param controller a progress controller. Call the
     * @ref PiiProgressController::canContinue() with no parameters time to
     * time to ensure you are still allowed to continue communication.
     * Returning from this function will automatically flush the
     * output pending in @p dev.
     *
     * @code
     * void MyHandler::handleRequest(const QString& uri, PiiHttpDevice* dev, TimeLimiter*)
     * {
     *   // Find the path of the request wrt to the "root" of this handler
     *   QString strRequestPath(dev->requestPath(uri));
     *   if (strRequestPath == "index.html" && dev->requestMethod() == "GET")
     *     dev->print("<html><head><title>Hello world!</title></head><body><!-- Secret message --></body></html>");
     * }
     * @endcode
     *
     * @exception The function may throw a PiiHttpException on error. 
     * %PiiHttpProtocol sets the response header correspondingly and
     * writes message to the response body.
     */
    virtual void handleRequest(const QString& uri, PiiHttpDevice* dev, TimeLimiter* controller) = 0;
  };
  
  PiiHttpProtocol();
  ~PiiHttpProtocol();
  
  void communicate(QIODevice* dev, PiiProgressController* controller);

  /**
   * Register a URI handler. He caller retains the ownership of the
   * handler. The same handler can be register many times in different
   * places. The @p uri parameter to the @ref
   * PiiHttpProtocol::UriHandler::handleRequest() "handleRequest()"
   * function tells the handler the URI it was registered at.
   *
   * The server will always look for the most specific handler. That
   * is, if you register handler A at "/" and handler B at "/myuri/",
   * every request beginning with "/myuri/" will be handled by B, and
   * every other request by A. Note that a request to "/myuri"
   * (without the trailing slash) will be served by A.
   *
   * @param uri the URI of the handler, relative to the server root.
   * Typically, slash-separated paths are used, but any valid URI
   * string will work. If a handler already exists at this URI, the
   * old handler will be replaced. If the URI does no start with a
   * slash, the function has no effect.
   *
   * @param handler the handler. When a request to the registered URI
   * is received, the handler will be invoked.
   *
   * @code
   * PiiHttpProtocol protocol;
   * // A handler that fetches files from the file system
   * PiiHttpFileSystemHandler* files = new PiiHttpFileSystemHandler("/var/www/html");
   * protocol.registerHandler("/", files);
   * // Use the WebDAV protocol to server requests to /dav and /repository
   * // Just an illustrative example, MyHttpDavHandler does not exist.
   * MyHttpDavHandler* dav = new MyHttpDavHandler("/home/dav/files");
   * protocol.registerHandler("/dav/", dav);
   * protocol.registerHandler("/repository/", dav);
   * @endcode
   *
   * Now, if a client requests "/dav/foobar", the handler named @p dav
   * will be invoked with "/dav/" as the @p uri parameter.
   */
  void registerUriHandler(const QString& uri, UriHandler* handler);

  /**
   * Get the handler (if any) that handles requests to @p uri. If @p
   * exactMatch is @p true, require an exact match. Otherwise find the
   * most specific match, even if not exact.
   *
   * @return the URI handler that serves requests to @p uri, or 0 if
   * no such handler exists.
   */
  UriHandler* uriHandler(const QString& uri, bool exactMatch=false);
  
  /**
   * Unregister a handler at @p uri.
   */
  void unregisterUriHandler(const QString& uri);

  /**
   * Unregister all occurrences of @p handler.
   */
  void unregisterUriHandler(UriHandler* handler);

  /**
   * Unregister all occurrences of @p handler. Note that it may not be
   * safe to delete the handler even if it has been unregistered. One
   * must first ensure that all connections have been terminated. It
   * is usually a good idea to shut down the server running the
   * protocol before deleting handlers.
   *
   * @param handler the handler that is to be unregistered in all
   * locations it has been registered to. If zero, the whole handler
   * registry will be cleared.
   */
  void unregisterAllHandlers(UriHandler* handler = 0);

  /**
   * Returns the status message for a numerical HTTP status code.
   *
   * @return a HTTP status string, such as "OK" (200) or "Moved
   * permanently" (301). If the code is not known, an empty string
   * will be returned.
   */
  static QString statusMessage(int code);

private:
  struct StatusCode;

  typedef QPair<QString, UriHandler*> HandlerPair;

  HandlerPair findHandler(const QString& path);
  
  /// @internal
  class Data : public PiiNetworkProtocol::Data
  {
  public:
    Data();
    QList<HandlerPair> lstHandlers;
    QMutex handlerListLock;
    int iMaxConnectionTime;
  };
  PII_D_FUNC;
};

#endif //_PIIHTTPPROTOCOL_H
