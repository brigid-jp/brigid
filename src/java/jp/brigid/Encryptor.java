package jp.brigid;

public class Encryptor {
  public Encryptor(String cipher, byte[] key, byte[] iv) {
    System.out.println("Encryptor(" + cipher + ")");
  }

  public long update(byte[] in, byte[] out, boolean padding) {
    System.out.println("Encryptor.update()");
    return 0;
  }
}
