// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

package jp.brigid;

import java.net.Authenticator;
import java.net.PasswordAuthentication;

public class HttpAuthenticator extends Authenticator {
  public HttpAuthenticator(int authScheme, String username, String password) {
    this.authScheme = authScheme;
    auth = new PasswordAuthentication(username, password.toCharArray());
  }

  public PasswordAuthentication getPasswordAuthentication() {
    String reqeustingScheme = getRequestingScheme().toLowerCase();
    switch (authScheme) {
      case 0: // none
        break;
      case 1: // basic
        if (reqeustingScheme.equals("basic")) {
          return auth;
        }
        break;
      case 2: // digest
        if (reqeustingScheme.equals("digest")) {
          return auth;
        }
        break;
      case 3: // any
        if (reqeustingScheme.equals("basic") || reqeustingScheme.equals("digest")) {
          return auth;
        }
        break;
    }
    return null;
  }

  private int authScheme;
  private PasswordAuthentication auth;
}
