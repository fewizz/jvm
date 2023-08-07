package java.util;

// TODO not thread-safe yet
public class Random {// implements RandomGenerator, Serializable

	private static long globalSeed = 0L;
	private long seed_;

	public Random() {
		this.seed_ = globalSeed++;
	}

	protected int next(int bits) {
		this.seed_ = (this.seed_ * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
		return (int)(this.seed_ >>> (48 - bits));
	}

	public int nextInt(int bound) {
		if (bound <= 0)
			throw new IllegalArgumentException("bound must be positive");
	 
		if ((bound & -bound) == bound)  // i.e., bound is a power of 2
			return (int)((bound * (long)next(31)) >> 31);
	 
		int bits, val;
		do {
			bits = next(31);
			val = bits % bound;
		} while (bits - val + (bound-1) < 0);
		return val;
	}

}