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

#ifndef _PIIMULTIPARTDECODER_H
#define _PIIMULTIPARTDECODER_H

#include <QIODevice>
#include <QStack>
#include "PiiMimeHeader.h"

/**
 * A class that decodes MIME multipart messages.
 *
 * %PiiMultipartDecoder is an IO device that decodes a multipart MIME
 * message and splits the input stream into chunks based on the
 * extracted information. Once a header has been fetched, read
 * operations will be terminated to the next boundary of the multipart
 * message body. The class recognizes nested multiparts.
 *
 * Consider the following multipart MIME message (example from
 * http://www.w3.org/TR/html401/interact/forms.html):
 *
@verbatim
Content-Type: multipart/form-data; boundary=AaB03x

--AaB03x
Content-Disposition: form-data; name="submit-name"

Larry
--AaB03x
Content-Disposition: form-data; name="files"
Content-Type: multipart/mixed; boundary=BbC04y

--BbC04y
Content-Disposition: file; filename="text.txt"
Content-Type: text/plain

... contents of text.txt ...
--BbC04y
Content-Disposition: file; filename="image.png"
Content-Type: image/png
Content-Transfer-Encoding: binary

...contents of image.png...
--BbC04y--
--AaB03x--

@endverbatim
 *
 * The code to read the message is as follows:
 *
 * @code
 * // Assume you have a tcp connection through "socket"
 * PiiMultipartDecoder decoder(socket);
 * while (decoder.nextMessage())
 *   {
 *     // Read body contents
 *     QByteArray aBody = decoder.readAll();
 *     if (decoder.header().contentType() == "image/png")
 *       decodePngImage(aBody);
 *   }
 * @endcode
 *
 * The decoder will read the topmost header first. Since the header
 * represents a multipart message, the header of the first body part
 * will also be read. After the first call to #bodyPartHeader(),
 * there will be two headers on the header stack (#depth()
 * will return 2). The first call to @p readAll() will return "Larry".
 *
 * The next round will also read two headers because the message
 * contains a nested multipart message (#depth() will return 3). The
 * second @p readAll() call will return the contents of file1.txt. The
 * third round fetches the contents of file2.gif, after which the loop
 * will break.
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiMultipartDecoder : public QIODevice
{
public:
  /**
   * Create a new multipart message decoder. This constructor assumes
   * that the message header has not been read yet.
   */
  PiiMultipartDecoder(QIODevice* device);
  /**
   * Create a new multipart message decoder with a header that has
   * already been read from the input device.
   *
   * @code
   * void MyHandler::handleRequest(const QString& uri,
   *                               PiiHttpDevice* h,
   *                               PiiProgressController* controller)
   * {
   *   if (!h->readHeader()) return;
   *   if (h->requestMethod() == "POST" &&
   *       h->requestHeader().contentType().startsWith("multipart/"))
   *     {
   *        PiiMultipartDecoder decoder(h, h->requestHeader());
   *        // Read message preamble
   *        decoder.readAll();
   *        while (decoder.nextMessage())
   *          {
   *            // ...
   *          }
   *     }
   * }
   * @endcode
   */
  PiiMultipartDecoder(QIODevice* device, const QHttpHeader& header);

  ~PiiMultipartDecoder();
  
  /**
   * Returns the header of a nested body part. The bottommost header
   * in the stack is either the one given in the constructor or the
   * first one read from the input strem. If the body is a multipart
   * message as of RFC 2387, more headers will be added to the stack
   * when the message body is being parsed. Once all body parts have
   * been read, the body part stack will be emptied.
   *
   * Since a multipart message can contain another multipart message,
   * the depth of the header stack can be arbitrary. The @p level
   * parameter can be used to obtain information about enclosing
   * messages. If there are no more body parts, or an incorrect @p
   * level is given, an invalid header will be returned.
   *
   * @param level the stacking level. 0 refers to the body part the
   * device is currently reading, 1 is its parent and so on.
   *
   * @return a MIME header or an invalid header if there is no such level.
   */
  PiiMimeHeader header(int level=0);

  /**
   * Reads headers from the input stream until an ordinary
   * (non-multipart) message header is found and puts all read headers
   * into the header stack. Headers can only be read once all the data
   * in a message body has been read.
   *
   * @return @p true if a message was successfully read, @p false if
   * no more messages could be read.
   *
   * @exception PiiMimeException& if the header is incorrectly encoded
   */
  bool nextMessage();

  /**
   * Returns the depth of the current body part stack. The depth
   * increases with each nesting level inside multipart messages. The
   * preamble of the first-level multipart message is at depth one,
   * and the first entity within the message is at depth two.
   */
  int depth();

  bool isSequential() const;
  qint64 bytesAvailable() const;
  
protected:
  /**
   * Reads data from the underlying device. Stops at message
   * boundaries.
   */
  qint64 readData(char* data, qint64 maxSize);
  /**
   * Writes data to the underlying device. Provided just for
   * completeness. Decoders aren't often used for writing.
   */
  qint64 writeData(const char* data, qint64 maxSize);
  
private:
  void popHeader();
  void updateBodyPartInfo();
  bool readPreamble();
  
  /// @internal
  class Data
  {
  public:
    Data(QIODevice* device);
    
    QIODevice* pDevice;
    bool bHeadersRead;
    QStack<PiiMimeHeader> stkHeaders;
    int iContentLength;
    int iCurrentMultipartDepth;
    QByteArray aBoundary, aBfr;
  } *d;
};

#endif //_PIIMULTIPARTDECODER_H
