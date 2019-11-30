// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

package jp.brigid;

import java.nio.ByteBuffer;
import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

public class AESEncryptor {
  public AESEncryptor(byte[] key, byte[] iv) throws Exception {
    cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
    cipher.init(Cipher.ENCRYPT_MODE, new SecretKeySpec(key, "AES"), new IvParameterSpec(iv));
  }

  public int update(ByteBuffer in, ByteBuffer out, boolean padding) throws Exception {
    if (padding) {
      return cipher.doFinal(in, out);
    } else {
      return cipher.update(in, out);
    }
  }

  private Cipher cipher;
}
