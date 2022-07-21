package java.io;

public class FileOutputStream {

	private final FileDescriptor fd_;

	public FileOutputStream(FileDescriptor fdObj) {
		this.fd_ = fdObj;
	}

	private static native int __write(int fd, int b);

	public void write(int b) throws IOException {
		if(__write(fd_.value_, b) == -1) {
			throw new IOException();
		}
	}

	public void write(byte[] b) throws IOException {
		write(b, 0, b.length);
	}

	public static native int __write_b(int fd, byte[] b, int off, int len);

	public void write(byte[] b, int off, int len) throws IOException {
		if(__write_b(fd_.value_, b, off, len) == -1) {
			throw new IOException();
		}
	}

	private static native int __close(int fd);

	public void close() throws IOException {
		if(__close(fd_.value_) == -1) {
			throw new IOException();
		}
	}

	public final FileDescriptor getFD() throws IOException {
		return fd_;
	}

}