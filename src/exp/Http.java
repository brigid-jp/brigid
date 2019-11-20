// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;

public class Http {
  public void http(String url) {
    try {
      URL u = new URL(url);
      HttpURLConnection connection = (HttpURLConnection)u.openConnection();
      connection.setRequestMethod("GET");
      connection.connect();
      int code = connection.getResponseCode();
      System.out.println("status_code " + code);

      System.out.println("content_type " + connection.getContentType());
      System.out.println("content_encoding " + connection.getContentEncoding());

      try (BufferedReader reader = new BufferedReader(new InputStreamReader(connection.getInputStream(), "UTF-8"))) {
        while (true) {
          String line = reader.readLine();
          if (line == null) {
            break;
          }
          System.out.println(line);
        }
      }
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  public static void main(String[] args) {
    String url = "https://brigid.jp/love2d-excersise/";
    if (args.length > 0) {
      url = args[0];
    }

    Http self = new Http();
    self.http(url);
  }
}
