// Copyright (c) 2019 <dev@brigid.jp>
// This software is released under the MIT License.
// https://opensource.org/licenses/mit-license.php

public class JNITest {
  static {
    System.loadLibrary("jnitest");
  }

  private native static int test();

  public static void main(String[] args) {
    System.exit(test());
  }
}
