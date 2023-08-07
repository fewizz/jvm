package java.lang;

import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.PrintStream;
import java.io.InputStream;
import java.nio.charset.Charset;

public final class System {

	public static final InputStream in
		= new FileInputStream(FileDescriptor.in);

	public static final PrintStream out = new PrintStream(
		new FileOutputStream(FileDescriptor.out), Charset.defaultCharset()
	);

	public static final PrintStream err = new PrintStream(
		new FileOutputStream(FileDescriptor.err), Charset.defaultCharset()
	);

	public static native long nanoTime();

	public static native void arraycopy(
		Object src, int srcPos, Object dest, int destPos, int length
	);

	public static void exit(int status) {
		Runtime.getRuntime().exit(status);
	}

}