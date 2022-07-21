package java.io;

public final class FileDescriptor {

	final int value_;

	public FileDescriptor() { // Why??
		this.value_ = -1;
	}

	FileDescriptor(int fd) {
		this.value_ = fd;
	}

	public boolean valid() {
		return this.value_ != -1;
	}

	public static native int __sync(int fd);

	public void sync() throws SyncFailedException {
		if(__sync(this.value_) == -1) {
			throw new SyncFailedException("Bad file descriptor");
		}
	}

	static final FileDescriptor
		err = new FileDescriptor(2),
		in  = new FileDescriptor(0),
		out = new FileDescriptor(1);

}