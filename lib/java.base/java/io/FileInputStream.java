package java.io;

public class FileInputStream extends InputStream {

	private final int fd_;

	public FileInputStream(FileDescriptor fdObj) {
		this.fd_ = fdObj.value_;
	}

	@Override
	native public int read() throws IOException;

}