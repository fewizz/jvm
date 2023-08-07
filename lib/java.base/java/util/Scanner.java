package java.util;

import java.io.InputStream;
import java.nio.charset.Charset;

public class Scanner { // implements Iterator<String>, Closeable

	private final InputStream is_;
	private final Charset charset_;

	public Scanner(InputStream source) {
		this.is_ = source;
		this.charset_ = Charset.defaultCharset();
	}

	public String next() {
		throw new RuntimeException();
	}

}