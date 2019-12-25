// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

package jp.brigid;

import java.nio.ByteBuffer;
import java.security.MessageDigest;

public class Hasher {
  public Hasher(byte[] algorithm) throws Exception {
    messageDigest = MessageDigest.getInstance(new String(algorithm, "UTF-8"));
  }

  public void update(ByteBuffer in) throws Exception {
    messageDigest.update(in);
  }

  public byte[] digest() throws Exception {
    return messageDigest.digest();
  }

  MessageDigest messageDigest;
}
