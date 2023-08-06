package java.io;

import java.nio.CharBuffer;

public abstract class Reader implements Readable, Closeable {

	protected Object lock;

	protected Reader() {
		this.lock = this;
	}

	protected Reader(Object lock) {
		if(lock == null) {
			throw new NullPointerException();
		}
		this.lock = lock;
	}

	@Override
	public native int read(CharBuffer target) throws IOException;

	public int read() throws IOException {
		char[] result = new char[1];
		int code = read(result, 0, 1);
		if(code == -1) {
			return -1;
		}
		return result[0];
	}

	public int read(char[] cbuf) throws IOException {
		return read(cbuf, 0, cbuf.length);
	}

	public abstract int read(char[] cbuf, int off, int len) throws IOException;

	public long skip(long n) throws IOException {
		long n0 = n;
		char[] tmp = new char[1]; // TODO we can do better =_=

		synchronized(lock) {
			while(n > 0) {
				int code = read(tmp, 0, 1);
				if(code == -1) {
					break;
				}
				--n;
			}
		}

		return n0 - n;
	}

	public boolean ready() throws IOException {
		return true;
	}

	public boolean markSupported() {
		return false;
	}

	public void mark(int readAheadLimit) throws IOException {
		throw new IOException("Unsupported");
	}

	public void reset() throws IOException {
		throw new IOException("Unsupported");
	}

	public abstract void close() throws IOException;

	public long transferTo(Writer out) throws IOException {
		if(out == null) {
			throw new NullPointerException();
		}

		char[] buff = new char[2048];
		int count;
		long total = 0;

		while((count = read(buff)) != -1) {
			out.write(buff, 0, count);
			total += count;
		}

		return total;
	}

}