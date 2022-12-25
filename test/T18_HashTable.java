package test;

import java.util.Hashtable;

class T18_HashTable {

	public static void main(String... args) {
		Hashtable<Integer, Float> ht = new Hashtable<Integer, Float>();

		if(ht.size() != 0) {
			System.exit(1);
		}

		Float prev = ht.put(new Integer(0), new Float(1.0F));
		if(prev != null) {
			System.exit(2);
		}
	}

}