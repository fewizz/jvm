package java.nio.charset;

public class CoderResult {

	static class Error extends CoderResult {
		@Override
		public boolean isError() {
			return true;
		}
	};

	static abstract class Malformed extends Error {
		@Override
		public boolean isMalformed() {
			return true;
		}

		@Override
		abstract public int length();
	}

	static class MalformedOfLength extends Malformed {
		int length_;

		MalformedOfLength(int length) {
			this.length_ = length;
		}

		@Override
		public int length() {
			return this.length_;
		}
	}

	public static final CoderResult OVERFLOW  = new Error();
	public static final CoderResult UNDERFLOW = new Error();
	public static final CoderResult MALFORMED_1 = new Malformed() {
		@Override
		public int length() {
			return 1;
		}
	};
	public static final CoderResult MALFORMED_2 = new Malformed() {
		@Override
		public int length() {
			return 2;
		}
	};
	public static final CoderResult MALFORMED_3 = new Malformed() {
		@Override
		public int length() {
			return 3;
		}
	};
	public static final CoderResult MALFORMED_4 = new Malformed() {
		@Override
		public int length() {
			return 4;
		}
	};

	public boolean isError() {
		return false;
	}

	public boolean isMalformed() {
		return false;
	}

	public int length() {
		throw new UnsupportedOperationException();
	}

}