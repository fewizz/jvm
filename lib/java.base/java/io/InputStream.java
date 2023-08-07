package java.io;

public abstract class InputStream { //implements Closeable {

	public InputStream() {}

	public abstract int read() throws IOException;

	public int read(byte[] b) throws IOException {
		return this.read(b, 0, b.length);
	}

	public int read(byte[] b, int off, int len) throws IOException {
		if(off < 0 || len < 0 || off + len > b.length) {
			throw new IndexOutOfBoundsException();
		}

		if(len == 0) return 0;

		int v = read();

		if(v == -1) {
			// "there is no more data
			//  because the end of the stream has been reached."
			return -1;
		}

		b[off + 0] = (byte) v;

		int x = 1;
		while(x < len) {
			try {
				v = read();
				if(v == -1) {
					break;
				}
				b[off + x] = (byte) v;
				++x;
			}
			catch(IOException e) { }
		}
		return x;

	}

}