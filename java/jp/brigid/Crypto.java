package jp.brigid;

import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

public class Crypto {

  public static byte[] encryptString(byte[] source, byte[] key, byte[] iv) {
    try {
      SecretKeySpec keySpec = new SecretKeySpec(key, "AES");
      IvParameterSpec ivSpec = new IvParameterSpec(iv);
      Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
      cipher.init(Cipher.ENCRYPT_MODE, keySpec, ivSpec);
      return cipher.doFinal(source);
    } catch (Exception e) {
      throw new RuntimeException(e);
    }
  }

  public static void main(String[] args) {
    try {
      String source = "The quick brown fox jumps over the lazy dog";
      String key = "01234567890123456789012345678901";
      String iv = "0123456789012345";

      byte[] result = encryptString(
          source.getBytes("UTF-8"),
          key.getBytes("UTF-8"),
          iv.getBytes("UTF-8"));

      int[] expect = {
        0xE0, 0x6F, 0x63, 0xA7, 0x11, 0xE8, 0xB7, 0xAA, 0x9F, 0x94, 0x40, 0x10, 0x7D, 0x46, 0x80, 0xA1,
        0x17, 0x99, 0x43, 0x80, 0xEA, 0x31, 0xD2, 0xA2, 0x99, 0xB9, 0x53, 0x02, 0xD4, 0x39, 0xB9, 0x70,
        0x2C, 0x8E, 0x65, 0xA9, 0x92, 0x36, 0xEC, 0x92, 0x07, 0x04, 0x91, 0x5C, 0xF1, 0xA9, 0x8A, 0x44,
      };

      if (result.length != expect.length) {
        System.out.println("invalid result length " + result.length);
        return;
      }

      for (int i = 0; i < result.length; ++i) {
        if (result[i] != (byte)expect[i]) {
          System.out.println("invalid result [" + i + "]");
          return;
        }
      }

    } catch (Exception e) {
      throw new RuntimeException(e);
    }
  }
}
