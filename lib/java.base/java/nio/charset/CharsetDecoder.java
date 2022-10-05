package java.nio.charset;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;

public abstract class CharsetDecoder {

	private final Charset charset_;
	private final float averageCharsPerByte_;
	private final float maxCharsPerByte_;
	private String replacement_ = "\uFFFD";

	private CodingErrorAction
		malformedInputAction_ = CodingErrorAction.REPORT,
		unmappableAction_ = CodingErrorAction.REPORT;

	protected CharsetDecoder(
		Charset cs, float averageCharsPerByte, float maxCharsPerByte
	) {
		this.charset_ = cs;
		this.averageCharsPerByte_ = averageCharsPerByte;
		this.maxCharsPerByte_ = maxCharsPerByte;
	}

	public final Charset charset() {
		return this.charset_;
	}

	public final String replacement() {
		return this.replacement_;
	}

	public final CharsetDecoder replaceWith(String newReplacement) {
		if(!(
			newReplacement != null &&
			newReplacement.length() > 0 &&
			newReplacement.length() <= this.maxCharsPerByte_
		)) {
			throw new IllegalArgumentException();
		}

		this.replacement_ = newReplacement;
		implReplaceWith(newReplacement);
		return this;
	}

	protected void implReplaceWith(String newReplacement) {}

	public CodingErrorAction malformedInputAction() {
		return this.malformedInputAction_;
	}

	public final CharsetDecoder onMalformedInput(CodingErrorAction newAction) {
		if(newAction == null) {
			throw new IllegalArgumentException();
		}

		this.malformedInputAction_ = newAction;
		implOnMalformedInput(newAction);
		return this;
	}

	protected void implOnMalformedInput(CodingErrorAction newAction) {}

	public CodingErrorAction unmappableCharacterAction() {
		return this.unmappableAction_;
	}

	public final CharsetDecoder onUnmappableCharacter(
		CodingErrorAction newAction
	) {
		if(newAction == null) {
			throw new IllegalArgumentException();
		}

		this.unmappableAction_ = newAction;
		implOnUnmappableCharacter(newAction);
		return this;
	}

	protected void implOnUnmappableCharacter(CodingErrorAction newAction) {}

	public final float averageCharsPerByte() {
		return this.averageCharsPerByte_;
	}

	public final float maxCharsPerByte() {
		return this.maxCharsPerByte_;
	}

	public final CoderResult decode(
		ByteBuffer in, CharBuffer out, boolean endOfInput
	) {
		CoderResult res = decodeLoop(in, out);
		if(res != CoderResult.UNDERFLOW) {
			return res;
		}
		if(endOfInput && in.remaining() > 0) {
			return new CoderResult.MalformedOfLength(in.remaining());
		}
		return res;
	}

	protected abstract CoderResult decodeLoop(ByteBuffer in, CharBuffer out);

}
