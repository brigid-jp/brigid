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
    this.method = decodeUTF8(method);
    connection.setRequestMethod(this.method);
    connection.setUseCaches(false);
  }

  public void setHeader(byte[] key, byte[] value) throws Exception {
    connection.setRequestProperty(decodeUTF8(key), decodeUTF8(value));
  }

  public void connect(long total) throws Exception {
    if (total >= 0) {
      connection.setDoOutput(true);
      connection.setFixedLengthStreamingMode(total);
    }

    connection.connect();

    if (total >= 0) {
      outputStream = connection.getOutputStream();
    }
  }

  public void write(byte[] buffer, int position, int size) throws Exception {
    outputStream.write(buffer, position, size);
  }

  public int getResponseCode() throws Exception {
    int code = connection.getResponseCode();

    if (!method.equals("HEAD")) {
      if (errorStream == null) {
        errorStream = connection.getErrorStream();
      }
      if (errorStream == null) {
        if (inputStream == null) {
          inputStream = connection.getInputStream();
        }
      }
    }

    return code;
  }

  public byte[] getHeaderFieldKey(int i) throws Exception {
    return encodeUTF8(connection.getHeaderFieldKey(i));
  }

  public byte[] getHeaderField(int i) throws Exception {
    return encodeUTF8(connection.getHeaderField(i));
  }

  public int read(byte[] buffer) throws Exception {
    if (errorStream != null) {
      return errorStream.read(buffer);
    } else if (inputStream != null) {
      return inputStream.read(buffer);
    } else {
      return -1;
    }
  }

  public void disconnect() throws Exception {
    if (outputStream != null) {
      outputStream.close();
    }
    if (inputStream != null) {
      inputStream.close();
    }
    if (errorStream != null) {
      errorStream.close();
    }
    connection.disconnect();
  }

  private HttpURLConnection connection;
  private String method;
  private OutputStream outputStream;
  private InputStream errorStream;
  private InputStream inputStream;
}
