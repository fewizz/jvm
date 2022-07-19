package java.nio;

public abstract class Buffer {

	private final int capacity_;
	private int limit_;
	private int position_ = 0;
	private int mark_ = -1;

	protected Buffer(int capacity) {
		this.capacity_ = capacity;
		this.limit_ = this.capacity_;
	}

	public final int capacity() {
		return capacity_;
	}

	public final int position() {
		return position_;
	}

	public Buffer position(int newPosition) {
		if(!(
			newPosition >= 0 && newPosition <= limit_
		)) {
			throw new IllegalArgumentException();
		}
		this.position_ = newPosition;
		return this;
	}

	public final int limit() {
		return limit_;
	}

	public Buffer limit(int newLimit) {
		if(!(
			newLimit >= 0 && newLimit <= capacity_
		)) {
			throw new IllegalArgumentException();
		}

		if(this.position_ > newLimit) {
			this.position_ = newLimit;
		}

		if(this.mark_ > newLimit) {
			this.mark_ = -1;
		}

		this.limit_ = newLimit;

		return this;
	}

	public Buffer mark() {
		this.mark_ = this.position_;
		return this;
	}

	public Buffer reset() {
		if(this.mark_ < 0) {
			throw new InvalidMarkException();
		}
		this.position_ = this.mark_;
		return this;
	}

	public Buffer clear() {
		this.position_ = 0;
		this.limit_ = this.capacity_;
		this.mark_ = -1;
		return this;
	}

	public Buffer flip() {
		this.limit_ = this.position_;
		this.position_ = 0;
		return this;
	}

	public Buffer rewind() {
		this.position_ = 0;
		this.mark_ = -1;
		return this;
	}

	public final int remaining() {
		return this.limit_ - this.position_;
	}

	public final boolean hasRemaining() {
		return remaining() > 0;
	}

	public abstract boolean isReadOnly();

	public abstract boolean hasArray();

	public abstract Object array();

	public abstract int arrayOffset();

	public abstract boolean isDirect();

	public abstract Buffer slice();

	public abstract Buffer slice(int index, int length);

	public abstract Buffer duplicate();

}