package java.nio.charset;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;

public abstract class CharsetEncoder {

	private final Charset charset_;
	private final float averageBytesPerChar_;
	private final float maxBytesPerChar_;
	private byte[] replacement_;

	private CodingErrorAction
		malformedInputAction_ = CodingErrorAction.REPORT,
		unmappableAction_ = CodingErrorAction.REPORT;

	protected CharsetEncoder(
		Charset cs, float averageBytesPerChar,
		float maxBytesPerChar, byte[] replacement
	) {
		if(!(
			averageBytesPerChar > 0.0F &&
			maxBytesPerChar > 0.0F &&
			replacement != null &&
			replacement.length != 0 &&
			replacement.length <= maxBytesPerChar &&
			isLegalReplacement(replacement)
		)) {
			throw new IllegalArgumentException();
		}

		this.charset_ = cs;
		this.averageBytesPerChar_ = averageBytesPerChar;
		this.maxBytesPerChar_ = maxBytesPerChar;
		this.replacement_ = replacement;
	}

	protected CharsetEncoder(
		Charset cs, float averageBytesPerChar, float maxBytesPerChar
	) {
		if(
			averageBytesPerChar <= 0.0F ||
			maxBytesPerChar <= 0.0F
		) {
			throw new IllegalArgumentException();
		}

		this.charset_ = cs;
		this.averageBytesPerChar_ = averageBytesPerChar;
		this.maxBytesPerChar_ = maxBytesPerChar;
		this.replacement_ = new byte[]{ (byte) '?' };
	}

	public final Charset charset() {
		return this.charset_;
	}

	public final byte[] replacement() {
		return this.replacement_;
	}

	public final CharsetEncoder replaceWith(byte[] newReplacement) {
		if(!(
			newReplacement != null &&
			newReplacement.length != 0 &&
			newReplacement.length <= maxBytesPerChar_ &&
			isLegalReplacement(newReplacement)
		)) {
			throw new IllegalArgumentException();
		}

		this.replacement_ = newReplacement;

		implReplaceWith(replacement_);

		return this;
	}

	protected void implReplaceWith(byte[] newReplacement) {}

	public boolean isLegalReplacement(byte[] repl) {
		// TODO
		return true;
	}

	public CodingErrorAction malformedInputAction() {
		return this.malformedInputAction_;
	}

	public final CharsetEncoder onMalformedInput(CodingErrorAction newAction) {
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

	public final CharsetEncoder onUnmappableCharacter(
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

	public final float averageBytesPerChar() {
		return this.averageBytesPerChar_;
	}

	public final float maxBytesPerChar() {
		return this.maxBytesPerChar_;
	}

	protected abstract CoderResult encodeLoop(CharBuffer in, ByteBuffer out);

}