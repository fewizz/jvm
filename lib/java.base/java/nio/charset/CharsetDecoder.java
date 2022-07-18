package java.nio.charset;

public abstract class CharsetDecoder {

	private final Charset charset_;
	private final float averageCharsPerByte_;
	private final float maxCharsPerByte_;

	protected CharsetDecoder(
		Charset cs, float averageCharsPerByte, float maxCharsPerByte
	) {
		this.charset_ = cs;
		this.averageCharsPerByte_ = averageCharsPerByte;
		this.maxCharsPerByte_ = maxCharsPerByte;
	}

	public final Charset charset() {
		return charset_;
	}

	public final float averageCharsPerByte() {
		return averageCharsPerByte_;
	}

	public final float maxCharsPerByte() {
		return maxCharsPerByte_;
	}

}
