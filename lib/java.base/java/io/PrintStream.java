package java.io;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetEncoder;

public class PrintStream extends FilterOutputStream {

	private final Charset charset_;
	private boolean error_ = false;
	private boolean flush_ = false;

	public PrintStream(OutputStream out, Charset charset) {
		super(out);
		this.charset_ = charset;
	}

	@Override
	public void flush() {
		try {
			super.flush();
		} catch(Exception e) {
			setError();
		}
	}

	@Override
	public void close() {
		try {
			super.close();
		} catch(Exception e) {
			setError();
		}
	}

	public boolean checkError() {
		return this.error_;
	}

	protected void setError() {
		this.error_ = true;
	}

	protected void clearError() {
		this.error_ = false;
	}

	@Override
	public void write(int b) {
		if(flush_ && b == '\n') {
			flush();
		}
		try {
			super.write(b);
		} catch(Exception e) {
			setError();
		}
	}

	@Override
	public void write(byte[] buf, int off, int len) {
		try {
			super.write(buf, off, len);
			if(flush_) {
				flush();
			}
		} catch(Exception e) {
			setError();
		}
	}

	@Override
	public void write(byte[] buf) throws IOException {
		try {
			super.write(buf);
			if(flush_) {
				flush();
			}
		} catch(Exception e) {
			setError();
		}
	}

	public void writeBytes(byte[] buf) {
		try {
			super.write(buf);
			if(flush_) {
				flush();
			}
		} catch(Exception e) {
			setError();
		}
	}

	public void print(boolean b) {
		char[] chars = b ?
			new char[]{ 't', 'r', 'u', 'e' } :
			new char[]{ 'f', 'a', 'l', 's', 'e' };
		print(chars);
	}

	public void print(char c) {
		print(new char[]{ c });
	}

	public void print(char[] s) {
		CharsetEncoder encoder = charset_.newEncoder();
		ByteBuffer result = ByteBuffer.allocate(
			(int)(encoder.maxBytesPerChar() * s.length + 1.0F)
		);
		encoder.encode(CharBuffer.wrap(s), result, true);
		write(result.array(), 0, result.position());
	}

	public void print(String s) {
		print(s.toCharArray());
	}

	public void println() {
		print(new char[]{ '\n' });
	}

	public void println(String x) {
		print(x);
		println();
	}

	public Charset charset() {
		return this.charset_;
	}

}