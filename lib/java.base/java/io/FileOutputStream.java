package java.io;

public class FileOutputStream extends OutputStream {

	private final int fd_value_;

	public FileOutputStream(FileDescriptor fdObj) {
		this.fd_value_ = fdObj.value_;
	}

	@Override
	native public void write(int b) throws IOException;

	@Override
	native public void write(byte[] b) throws IOException;

	@Override
	native public void write(byte[] b, int off, int len) throws IOException;

	@Override
	native public void flush() throws IOException;

	@Override
	native public void close() throws IOException;

	public final FileDescriptor getFD() throws IOException {
		return new FileDescriptor(fd_value_);
	}

}