package java.io;

public class FileOutputStream {

	private final FileDescriptor fd_;

	public FileOutputStream(FileDescriptor fdObj) {
		this.fd_ = fdObj;
	}

	private static native boolean write(long fd, int b);

	public void write(int b) throws IOException {
		if(!write(fd_.value_, b)) {
			throw new IOException();
		}
	}

	public void write(byte[] b) throws IOException {
		write(b, 0, b.length);
	}

	public static native boolean write_buffer(
		long fd, byte[] b, int off, int len
	);

	public void write(byte[] b, int off, int len) throws IOException {
		if(!write_buffer(fd_.value_, b, off, len)) {
			throw new IOException();
		}
	}

	private static native boolean close(long fd);

	public void close() throws IOException {
		if(!close(fd_.value_)) {
			throw new IOException();
		}
	}

	public final FileDescriptor getFD() throws IOException {
		return fd_;
	}

}