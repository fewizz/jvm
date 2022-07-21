package java.io;

public abstract class Writer
	implements Appendable, Closeable, Flushable
{

	protected Object lock;

	protected Writer() {
		this.lock = this;
	}

	protected Writer(Object lock) {
		if(lock == null) {
			throw new NullPointerException();
		}
		this.lock = lock;
	}

	public void write(int c) throws IOException {
		write(new char[]{ (char) c }, 0, 1);
	}

	public void write(char[] cbuf) throws IOException {
		write(cbuf, 0, cbuf.length);
	}

	public abstract void write(
		char[] cbuf, int off, int len
	) throws IOException;

	public void write(String str, int off, int len) throws IOException {
		write(str.toString(), 0, str.length());
	}

	public void write(String str) throws IOException {
		write(str.toString());
	}

	@Override
	public Writer append(CharSequence csq) throws IOException {
		write(csq.toString());
		return this;
	}

	@Override
	public Writer append(
		CharSequence csq, int start, int end
	) throws IOException {
		write(csq.subSequence(start, end).toString());
		return this;
	}

	@Override
	public Writer append(char c) throws IOException {
		write(c);
		return this;
	}

	@Override
	public abstract void flush() throws IOException;

	@Override
	public abstract void close() throws IOException;

}