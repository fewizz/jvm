package java.lang;

import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.nio.charset.Charset;

public final class System {

	public static final PrintStream out = new PrintStream(
		new FileOutputStream(FileDescriptor.out), Charset.defaultCharset()
	);

	public static native long nanoTime();

	public static native void arraycopy(
		Object src, int srcPos, Object dest, int destPos, int length
	);

	public static void exit(int status) {
		Runtime.getRuntime().exit(status);
	}

}