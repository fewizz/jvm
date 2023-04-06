import java.util.Hashtable;

class T18_HashTable {

	public static void main(String... args) {
		Hashtable<Integer, Float> ht = new Hashtable<Integer, Float>(1);

		if(ht.size() != 0) {
			System.exit(1);
		}

		Float prev = ht.put(0, 1.0F);
		if(prev != null) {
			System.exit(2);
		}

		if(!ht.containsKey(0)) {
			System.exit(3);
		}
		if(!ht.containsValue(1.0F)) {
			System.exit(4);
		}

		prev = ht.put(0, 42.0F);
		if(prev == null) {
			System.exit(5);
		}
		if(prev != 1.0F) {
			System.exit(6);
		}
		if(ht.size() != 1) {
			System.exit(7);
		}

		ht.put(10, 10.0F);

		if(ht.size() != 2) {
			System.exit(8);
		}

		if(!ht.containsKey(0)) {
			System.exit(9);
		}
		if(ht.get(0) != 42.0F) {
			System.exit(10);
		}

		if(!ht.containsKey(10)) {
			System.exit(11);
		}
		if(ht.get(10) != 10.0F) {
			System.exit(12);
		}

		for(int i = 0; i < 100; ++i) {
			ht.put(i, (float) (i * 2));
		}

		if(ht.size() != 100) {
			System.exit(13);
		}

		for(int i = 0; i < 100; ++i) {
			if(ht.get(i) != (float)(i * 2)) {
				System.exit(14);
			}
		}
	}

}