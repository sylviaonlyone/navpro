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

#ifndef _PIIHTTPDEVICE_H
#define _PIIHTTPDEVICE_H

#include <QIODevice>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>
#include <QVariantMap>
#include <QStack>
#include <QVariant>

#include <PiiTimer.h>

#include "PiiSocketDevice.h"
#include "PiiStreamFilter.h"
#include "PiiHttpBodyPartHeader.h"

class QTextCodec;
class PiiProgressController;

/**
 * An I/O device for HTTP/1.1 communication. This class can be used to
 * send and parse HTTP request and replies. It works as an ordinary
 * I/O device, but ensures that the HTTP protocol format is retained.
 *
 * The class automates the handling of HTTP headers as much as
 * possible. If you request information that is not available without
 * reading the request header, @p %PiiHttpDevice will read the header
 * automatically. It also builds the request or response header, and
 * has reasonable defaults for most header fields. The header will be
 * automatically sent if data is written to the device.
 *
 * @p %PiiHttpDevice supports output filtering. All data written to
 * the client/server can be passed through an unlimited number of
 * filters which may buffer the data. If no filters are installed,
 * data will be written directly to the low-level socket. Headers will
 * be sent just before the first byte of the message body is going to
 * be written to the client. It is not possible to change the headers
 * afterwards.
 *
 * In @p Server mode, @p %PiiHttpDevice is typically used in the @p
 * handleRequest() function of PiiHttpProtocol::UriHandler:
 *
 * @code
 * void MyHandler::handleRequest(const QString& uri,
 *                               PiiHttpDevice* h,
 *                               PiiProgressController* controller)
 * {
 *   if (h->requestMethod() != "GET")
 *     {
 *       h->setStatus(PiiHttpProtocol::MethodNotAllowedStatus);
 *       return; // do not send anything
 *     }
 *   h->startOutputFiltering(new PiiStreamBuffer);
 *   h->setHeader("Content-Type", "text/html");
 *   h->print("<html><body>You hit a handler at " + uri + "</body></html>\n");
 * }
 * @endcode
 *
 * Using a PiiStreamBuffer as an output filter will usually increase
 * performance. The effect is that writes to the device will be stored
 * into memory until the buffer is flushed. PiiHttpProtocol can also
 * utilizes the buffer by automatically setting the Content-Length
 * header.
 *
 * In @p Client mode, the I/O device must be created first. Note that
 * %PiiHttpDevice assumes that the I/O device can always read input
 * data. It doesn't implement any waiting mechanisms for the cases
 * where data is not immediately available. This makes communication
 * devices such as QTcpSocket unsuitable as such. Use
 * PiiWaitingIODevice instead. PiiNetworkClient can be used to easily
 * create a suitable I/O device:
 *
 * @code
 * PiiNetworkClient client("tcp://127.0.0.1:80");
 * PiiHttpDevice dev(client.openConnection(), PiiHttpDevice::Client);
 * dev.setRequest("GET", "/");
 * dev.setHeader("Host", "localhost");
 * dev.finish();
 *
 * // Read response header and ignore the response body.
 * dev.readHeader();
 * dev.discardBody();
 * @endcode
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiHttpDevice : public QIODevice, public PiiStreamFilter
{
  Q_OBJECT
  
public:
  /**
   * Connection types, specified by the @p Connection request header.
   *
   * @lip KeepAliveConnection - the default for HTTP/1.1. The
   * connection will not be closed after a request.
   *
   * @lip CloseConnection - the connection will be closed after
   * request. This happens when the client sends the "Connection:
   * close" request header.
   */
  enum ConnectionType { KeepAliveConnection, CloseConnection };

  /**
   * Communication modes.
   *
   * @lip Client - the device is used at the client side
   *
   * @lip Server - the device is used at the server side
   */
  enum Mode { Client, Server };

  /**
   * Encoding formats.
   *
   * @lip TextFormat - data is encoded as UTF-8 text. See
   * PiiTextOutputArchive and PiiTextInputArchive.
   *
   * @lip BinaryFormat - data is encoded in a raw binary format. See
   * PiiBinaryOutputArchive and PiiBinaryInputArchive.
   */
  enum EncodingFormat { TextFormat, BinaryFormat };

  /**
   * Initialize a HTTP device with the given low-level device.
   *
   * @param device the communication device. Typically, @a device is a
   * PiiWaitingIODevice. The pointer must remain valid during the
   * lifetime of the HTTP device.
   *
   * @param mode communication mode
   */
  PiiHttpDevice(const PiiSocketDevice& device, Mode mode);

  /**
   * Destroy the HTTP device. This will finish all unfinished output
   * filters.
   */
  ~PiiHttpDevice();
  
  /**
   * Returns the request method. Typically, the answer is either "GET" or
   * "POST", but HTTP extensions may use arbitrary request methods.
   */
  QString requestMethod() const;

  /**
   * Returns the connection type. If the client has the "Connection"
   * header set to "close", or if the client disconnects, this
   * function returns @p CloseConnection. Otherwise it returns @p
   * KeepAliveConnection.
   */
  ConnectionType connectionType() const;

  /**
   * Returns the request URI. The returned string contains the full
   * path the client sent with the request, including possible encoded
   * request parameters.
   */
  QString requestUri() const;

  /**
   * Returns the query string part of the request URI, if any. The
   * query string contains URI-encoded query values. The question mark
   * at the beginning of the query string will not be returned.
   *
   * @return the query string. May be empty.
   *
   * @code
   * // request URI: /foo/bar/baz?a=2
   * QString str = h->queryString();
   * // str = "a=2"
   * @endcode
   */
  QString queryString() const;

  /**
   * Returns @p true if the request URI contains a query string, and
   * @p false otherwise.
   */
  bool hasQuery() const;

  /**
   * Returns a query value from the request URI. This function returns
   * the variable called @a name in the encoded request string. If
   * there are multiple values associated with the same key, a list of
   * values will be returned. Values are automatically decoded (see
   * #decodeVariant()).
   *
   * @code
   * // request URI: /path/?var1=a&var2=b&var2=c
   *
   * QString a = h->queryValue("var1").toString();
   * // a = "a"
   * QVariantList bc = h->queryValue("var2").toList();
   * // bc = ("b", "c")
   * @endcode
   */
  QVariant queryValue(const QString& name) const;

  /**
   * Returns all query values as a map of key-value pairs. Parameter
   * values are automatically decoded (see #decodeVariant()).
   */
  QVariantMap queryValues() const;

  /**
   * Returns the names of all query items as a list. This function
   * returns the "raw" query names in the order they appear in the
   * request URI. The list may contain duplicate entries.
   */
  QStringList queryItems() const;

  /**
   * Adds a query value to the request URI. This function can be used
   * in @p Client mode to automatically encode values in the query.
   *
   * @param name the name of the query value. If the name is an empty
   * string, the name and the equal sign will be omitted.
   *
   * @param value the value, will be automatically encoded (see
   * #encode()). Multiple items with the same name may exists.
   *
   * @code
   * dev.addQueryValue("v1", 1);
   * dev.addQueryValue("v2", "Hello");
   * dev.addQueryValue("v2", 3.14);
   *
   * // dev.queryString() == v1=1&v2=%22hello%22&v2=3.14
   * @endcode
   */
  void addQueryValue(const QString& name, const QVariant& value);

  /**
   * Remove a query value from the request URI.
   */
  void removeQueryValue(const QString& name);

  /**
   * Remove all query values from the request uri.
   */
  void removeQueryValues();

  /**
   * Returns the path part of the request URI.
   *
   * @param basePath strip this path from the beginning of the path. 
   * This is useful if you want to find the relative path of a URI
   * handler.
   *
   * @code
   * // request URI: /foo/bar/baz?a=2
   * QString str = h->requestPath("/foo/");
   * // str = "bar/baz"
   * @endcode
   */
  QString requestPath(const QString& basePath = QString()) const;

  /**
   * Set a value in the request header.
   *
   * @param key the name of the header field, such as "Content-Type".
   *
   * @param value the value of the field
   *
   * @param replace indicates whether the header should replace a
   * previous similar header, or add a second header of the same type.
   */
  void setRequestHeader(const QString& name, const QString& value, bool replace);

  /**
   * Returns the full request header.
   */
  QHttpRequestHeader requestHeader() const;

  /**
   * Set a value in the response header.
   *
   * @param key the name of the header field, such as "Location".
   *
   * @param value the value of the field
   *
   * @param replace indicates whether the header should replace a
   * previous similar header, or add a second header of the same type.
   *
   * @note The function will automatically change the status code
   * (#setStatus()) if needed. For example, setting the @p Location
   * header changes the status code automatically to 302 (redirect),
   * unless the status code is already in the 3xx series.
   */
  void setResponseHeader(const QString& name, const QString& value, bool replace);

  /**
   * Returns the full response header.
   */
  QHttpResponseHeader responseHeader() const;

  /**
   * Set the request method to @a method and request URI to @a uri. 
   * The @a uri will not be parsed; you can give anything (including
   * invalid data) as the request uri. Calling this function will
   * clear all existing parameters added with #addQueryValue().
   */
  void setRequest(const QString& method, const QString& uri);

  /**
   * Returns a request parameter from an
   * @p application/x-www-form-urlencoded POST body. If the body is not
   * such type, or the value called @a name does not exist, an invalid
   * variant will be returned. Note that this function will only work
   * if you haven't already read the message body yourself.
   *
   * @note If the message body has not been read, this function does
   * so, even though it is @p const.
   *
   * @see #queryValue()
   */
  QVariant formValue(const QString& name) const;

  /**
   * Returns all form-encoded values in the message body as a map. 
   * Parameter values are automatically decoded (see
   * #decodeVariant()).
   *
   * @note If the message body has not been read, this function does
   * so, even though it is @p const.
   */
  QVariantMap formValues() const;

  /**
   * Returns the names of form fields in the order they appear in the
   * POST message body.
   *
   * @note If the message body has not been read, this function does
   * so, even though it is @p const.
   *
   * @see queryItems()
   */
  QStringList formItems() const;

  /**
   * Returns a request parameter from either the request URI (GET
   * parameters) or the request body (POST parameters). If the
   * variable is found in both, the request URI takes precedence.
   */
  QVariant requestValue(const QString& name) const;

  /**
   * Returns all request parameters (either GET parameters in the URI
   * or form values in the message body) as a map. If the same
   * variable is found in both, the request URI takes precedence. 
   * Parameter values are automatically decoded (see
   * #decodeVariant()).
   */
  QVariantMap requestValues() const;

  /**
   * Returns the client-supplied name of the host, if there is one.
   */
  QString host() const;

  /**
   * Returns the HTTP response status code.
   */
  int status() const;

  /**
   * Set the HTTP status code. The default value is 200 (OK).
   */
  void setStatus(int code);

  /**
   * Puts an output filter on the filter stack. All subsequent writes
   * to the device will go through @a filter first.
   *
   * You may call startOutputFiltering() many times. Just make sure
   * that you call #endOutputFiltering() the same number of times. If
   * multiple output filters are active, output will be filtered
   * sequentially through each of them.
   *
   * @param filter the output filter. %PiiHttpDevice takes the
   * ownership of the pointer.
   *
   * @code
   * // Buffer all data into memory until finish() or endOutputFiltering() is called.
   * dev.startOutputFiltering(new PiiStreamBuffer);
   * @endcode
   */
  void startOutputFiltering(PiiStreamFilter* filter);

  /**
   * Returns the currently active (topmost) output filter.
   *
   * @return the filter that first filters data written to the device,
   * or 0 if no filters have been added.
   */
  PiiStreamFilter* outputFilter() const;
  
  /**
   * Pops an output filter from the filter stack. This function will
   * collapse the filter stack until it finds @a filter and tell each
   * filter to finish its job. The removed filters will be deleted. 
   * The next filter below @a filter on the stack (if any) will be
   * activated.
   *
   * If the last filter is removed, and the filter has buffered data,
   * this function tries to determine the number of bytes to be sent
   * to the client and set the Content-Length header accordingly. If a
   * single byte has been sent previously, it is too late to send the
   * header. If you want to avoid sending the automatic header, call
   * filter->flushFilter() to flush buffered data before
   * endOutputFiltering().
   *
   * @param filter the filter until which all filters will be
   * finished. If @a filter is 0, only the topmost filter will be
   * removed.
   */
  void endOutputFiltering(PiiStreamFilter* filter = 0);

  /**
   * Sets a HTTP request/response header field. If the device is in
   * @p Client mode, this function modifies the request header. In
   * @p Server mode, it modifies the response header.
   *
   * @param name the name of the header field, such as "Location".
   *
   * @param value the value of the field
   *
   * @param replace indicates whether the header should replace a
   * previous similar header, or add a second header of the same type.
   *
   * @code
   * // Set content type
   * h->setHeader("Content-Type", "text/xml");
   *
   * // Set content length
   * h->setHeader("Content-Length", 31416);
   *
   * // Redirect the client to another address
   * h->setHeader("Location", "http://www.intopii.fi/");
   * // Note: it is not a good idea to send data to the client after
   * // redirecting it.
   *
   * // Set authentication style
   * h->setHeader("WWW-Authenticate" "Negotiate");
   * // Add another header with the same name
   * h->setHeader("WWW-Authenticate", "NTLM", false);
   * @endcode
   *
   * @note Setting the @p Content-Encoding header will automatically
   * change the output text codec. See QTextCodec for supported
   * encodings.
   */
  void setHeader(const QString& name, const QVariant& value, bool replace = true);

  /**
   * Removes the header field(s) with @a name. If the device is in
   * @p Client mode, this function modifies the request header. In
   * @p Server mode, it modifies the response header.
   */
  void removeHeader(const QString& name);

  /**
   * Returns the length of the request/response body.
   */
  int contentLength() const;

  /**
   * See if request/response header have been successfully read.
   *
   * @return @p true, if request headers have been read, @p false
   * otherwise.
   */
  bool headerRead() const;
  
  /**
   * Set the maximum number of bytes an HTTP request header is allowed
   * to contain. This value is used to prevent exhaustive memory
   * allocation. The default value is 4096.
   */
  void setHeaderSizeLimit(qint64 headerSizeLimit);
  /**
   * Returns the current header size limit.
   */
  qint64 headerSizeLimit() const;
  /**
   * Set the maximum number of bytes the entire HTTP message
   * (including the header and the body) is allowed to contain. This
   * value is used to prevent exhaustive memory allocation. The
   * default value is 1M.
   */
  void setMessageSizeLimit(qint64 messageSizeLimit);
  /**
   * Returns the current request size limit.
   */
  qint64 messageSizeLimit() const;

  /**
   * Ensures that headers are sent to the other end of the
   * connections, flushes all output filters and flushes the output
   * device. On the client side, this function must be called before
   * reading a response. The final form of the request/response
   * headers is available only after finish() is done. For
   * example, #connectionType() may be @p CloseConnection even if the
   * client did not request it.
   *
   * Calling finish() multiple times has no effect. If you intend to
   * use the same %PiiHttpDevice for successive requests, you need to
   * call either #restart() after handling the request. The
   * #setRequest() (on the client side) and #readHeader() (on the
   * server side) functions automatically restart the request.
   */
  void finish();

  qint64 flushFilter();

  /**
   * Restarts the request/reply cycle. This function needs to be
   * called after each cycle if the same device is used again. On the
   * client side, each #setRequest() call automatically restarts the
   * cycle. On the server side, each #readHeader() call does the same.
   */
  void restart();
  
  /**
   * Returns @p true if the low-level socket device is still
   * connected, @p false otherwise.
   */
  bool isWritable() const;

  /**
   * Returns @p true if the low-level socket device is still
   * connected, @p false otherwise.
   */
  bool isReadable() const;

  /**
   * Prints text to the device. This function is equivalent to
   * QIODevice::write(), but it converts the unicode input text using
   * the encoding style defined with the @p Content-Encoding header. 
   * If no encoding has been set, UTF-8 will be used.
   */
  qint64 print(const QString& data);

  /**
   * Encodes @a msg to a byte array using the current encoding. If the
   * @p Content-Encoding header has not been set, UTF-8 will be used.
   */
  QByteArray encode(const QString& msg) const;

  qint64 filterData(const char* data, qint64 maxSize);
  
  bool isSequential() const;
  qint64 bytesAvailable() const;

  /**
   * Reads the whole message body and returns it as a byte array. Note
   * that the function returns data even if the whole body could not
   * be read.
   *
   * @note If you call this function yourself, a POST encoded message
   * body can no longer be automatically parsed, and functions such as
   * #formItems() and #formValues() return empty values.
   */
  QByteArray readBody();

  /**
   * Reads and forgets the whole message body.
   *
   * @note If you call this function yourself, a POST encoded message
   * body can no longer be automatically parsed, and functions such as
   * #formItems() and #formValues() return empty values.
   */
  void discardBody();
  
  /**
   * Reads request/response header. This function checks that the
   * header has not been read and calls the protected #decodeHeader()
   * function if needed.
   */
  bool readHeader();

  /**
   * Sends request/response headers. This function checks that the
   * headers have not been sent and calls the protected
   * #encodeHeader() function if needed.
   */
  bool sendHeader();

  /**
   * Returns @p true if at least one byte of the message body has been
   * read and @p false otherwise.
   */
  bool isBodyRead() const;

  /**
   * Returns the number of bytes in the message body. This value is
   * known only if the header contains a Content-Length field. 
   * Otherwise -1 will be returned.
   */
  qint64 bodyLength() const;
  /**
   * Returns the number of bytes in the parsed request/response
   * header. If the header has not been read, -1 will be returned.
   */
  qint64 headerLength() const;

  /**
   * Decodes @a data and returns its value as a %QVariant. The
   * following conversions are tried, in preference order:
   *
   * @li If @a data begins with a magic string that identifies an
   * archive (e.g. "Into Txt" or "Into Bin"), it is assumed to contain
   * a serialized QVariant. The variant will be deserialized.
   *
   * @li If @a data can be converted to an @p int, an @p int will be
   * returned.
   *
   * @li If @a data can be converted to a @p double, a @p double will
   * be returned.
   *
   * @li If @a data is either "true" or "false", the corresponding
   * boolean value will be returned.
   *
   * @li Otherwise, a QString will be returned. If the value is
   * enclosed in double quotes, they will be removed.
   *
   * @note In some occasions, it may be necessary to pass "true",
   * "false", "1.23" or "Into Bin" as strings. To prevent automatic
   * conversion, enclose the value in double quotes.
   *
   * @return the decoded value. If the decoding fails, an invalid
   * variant will be returned.
   */
  QVariant decodeVariant(const QByteArray& data) const;
  /// @overload
  QVariant decodeVariant(const QString& data) const;

  /**
   * Encodes a QVariant to a QByteArray. If the variant represents a
   * number, it will be converted to decimal representation. Boolean
   * values will be converted to "true" and "false" (without the
   * double quotes). QStrings will be enclosed in double quotes. All
   * other types will be serialized.
   */
  QByteArray encode(const QVariant& variant, EncodingFormat format = TextFormat) const;

  /**
   * Set the communication device. Usually, a new %PiiHttpDevice is
   * created whenever a new communication device is needed. This
   * function is useful in @p Client mode if you need to reopen a
   * connection to a server but still want to retain old configuration
   * such as request parameters or cookies. Setting @a device to null
   * is not allowed.
   *
   * @exception PiiInvalidArgumentException& if @a device is null.
   */
  void setDevice(const PiiSocketDevice& device);
  
  /**
   * Returns the communication device. Note that writing to the device
   * directly may break the HTTP protocol. Use the underlying device
   * at your own risk.
   */
  PiiSocketDevice device() const;

  void setController(PiiProgressController* controller);
  PiiProgressController* controller() const;
  
protected:
  qint64 readData(char* data, qint64 maxSize);
  qint64 writeData(const char * data, qint64 maxSize);
  
private:
  void parseQueryValues(const QString& uri);
  void parseVariables(const QByteArray& data, QVariantMap& valueMap, QStringList& names);
  inline void addToMap(QVariantMap& map, const QString& key, const QByteArray& value);
  void addToMap(QVariantMap& map, const QString& key, const QVariant& value);
  template <class Archive> static QVariant decodeVariant(const QByteArray& data);
  template <class Archive> static QByteArray encode(const QVariant& variant);

  inline qint64 writeToSocket(const char * data, qint64 maxSize);
  void checkCodec(const QString& key, const QString& value);

  void readFormValues();

  void createQueryString();
  bool sendResponseHeader();
  bool decodeResponseHeader();
  bool sendRequestHeader();
  bool decodeRequestHeader();

  /// @internal
  class Data : public PiiStreamFilter::Data
  {
  public:
    Data(PiiHttpDevice* owner, const PiiSocketDevice& device, Mode mode);
    
    Mode mode;
    
    QVariantMap mapQueryValues;
    QStringList lstQueryItems;
    QVariantMap mapFormValues;
    QStringList lstFormItems;
    QHttpRequestHeader requestHeader;
    QHttpResponseHeader responseHeader;
    
    PiiSocketDevice pSocket;
    PiiProgressController* pController;
    PiiStreamFilter* pActiveOutputFilter;
    QTextCodec* pTextCodec;
    bool bHeaderRead, bHeaderSent;
    qint64 iHeaderSizeLimit;
    qint64 iMessageSizeLimit;
    qint64 iBytesRead, iBytesWritten;
    bool bBodyRead, bFinished;
    qint64 iBodyLength, iHeaderLength;
  };
  PII_D_FUNC;
};

#endif //_PIIHTTPDEVICE_H
