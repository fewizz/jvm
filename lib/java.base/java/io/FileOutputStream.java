package java.io;

public class FileOutputStream {

	private final int fd_value_;

	public FileOutputStream(FileDescriptor fdObj) {
		this.fd_value_ = fdObj.value_;
	}

	native public void write(int b) throws IOException;

	native public void write(byte[] b) throws IOException;

	native public void write(byte[] b, int off, int len) throws IOException;

	native public void close() throws IOException;

	public final FileDescriptor getFD() throws IOException {
		return new FileDescriptor(fd_value_);
	}

}