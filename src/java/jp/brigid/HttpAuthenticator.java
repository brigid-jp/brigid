// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

package jp.brigid;

import java.net.Authenticator;
import java.net.PasswordAuthentication;

public class HttpAuthenticator extends Authenticator {
  public HttpAuthenticator(String username, String password) {
    auth = new PasswordAuthentication(username, password.toCharArray());
  }

  protected PasswordAuthentication getPasswordAuthentication() {
    return auth;
  }

  private PasswordAuthentication auth;
}
