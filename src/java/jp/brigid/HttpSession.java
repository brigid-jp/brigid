// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

package jp.brigid;

import java.nio.ByteBuffer;

public class HttpSession {
  public HttpSession(int authScheme, byte[] username, byte[] password) throws Exception {
    this.authScheme = authScheme;
    this.username = new String(username, "UTF-8");
    this.password = new String(password, "UTF-8");
  }

  public void request(
      

  private int authScheme;
  private String username;
  private String password;
}
