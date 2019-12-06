// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

package jp.brigid;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.Authenticator;
import java.net.HttpURLConnection;
import java.net.URL;

public class HttpTask {
  private static byte[] encodeUTF8(String source) throws Exception {
    if (source == null) {
      return null;
    } else {
      return source.getBytes("UTF-8");
    }
  }

  private static String decodeUTF8(byte[] source) throws Exception {
    return new String(source, "UTF-8");
  }

  public static void setCredential(byte[] username, byte[] password) throws Exception {
    Authenticator.setDefault(new HttpAuthenticator(decodeUTF8(username), decodeUTF8(password)));
  }

  public static void resetCredential() {
    Authenticator.setDefault(null);
  }

  public HttpTask(byte[] method, byte[] url) throws Exception {
    connection = (HttpURLConnection) new URL(decodeUTF8(url)).openConnection();
    connection.setRequestMethod(decodeUTF8(method));
    connection.setUseCaches(false);
  }

  public void setHeader(byte[] key, byte[] value) throws Exception {
    connection.setRequestProperty(decodeUTF8(key), decodeUTF8(value));
  }

  public void send() throws Exception {
    connection.connect();
  }

  public void sendBody(long total) throws Exception {
    connection.setDoOutput(true);
    connection.setFixedLengthStreamingMode(total);
    connection.connect();
    outputStream = connection.getOutputStream();
  }

  public void write(byte[] buffer, int position, int size) throws Exception {
    outputStream.write(buffer, position, size);
  }

  public int recv() throws Exception {
    if (outputStream != null) {
      outputStream.close();
      outputStream = null;
    }

    int code = connection.getResponseCode();

    if (!connection.getRequestMethod().equals("HEAD")) {
      inputStream = connection.getErrorStream();
      if (inputStream == null) {
        inputStream = connection.getInputStream();
      }
    }

    return code;
  }

  public int getResponseCode() throws Exception {
    return connection.getResponseCode();
  }

  public byte[] getHeaderKey(int i) throws Exception {
    return encodeUTF8(connection.getHeaderFieldKey(i));
  }

  public byte[] getHeaderValue(int i) throws Exception {
    return encodeUTF8(connection.getHeaderField(i));
  }

  public int read(byte[] buffer) throws Exception {
    if (inputStream != null) {
      return inputStream.read(buffer);
    } else {
      return -1;
    }
  }

  public void close() throws Exception {
    if (outputStream != null) {
      outputStream.close();
      outputStream = null;
    }
    if (inputStream != null) {
      inputStream.close();
      inputStream = null;
    }
    if (connection != null) {
      connection.disconnect();
      connection = null;
    }
  }

  private HttpURLConnection connection;
  private OutputStream outputStream;
  private InputStream inputStream;
}
