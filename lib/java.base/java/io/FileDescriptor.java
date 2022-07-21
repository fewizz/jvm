package java.io;

public final class FileDescriptor {

	private final int fd_;

	public FileDescriptor() { // Why??
		this.fd_ = -1;
	}

	private FileDescriptor(int fd) {
		this.fd_ = fd;
	}

	public boolean valid() {
		return this.fd_ != -1;
	}

	public static native int __sync(int fd);

	public void sync() throws SyncFailedException {
		if(__sync(this.fd_) == -1) {
			throw new SyncFailedException("Bad file descriptor");
		}
	}

	static final FileDescriptor
		err = new FileDescriptor(2),
		in  = new FileDescriptor(0),
		out = new FileDescriptor(1);

}