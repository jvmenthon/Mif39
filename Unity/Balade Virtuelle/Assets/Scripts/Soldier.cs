using UnityEngine;
using System.Collections;

public class Soldier : MonoBehaviour {

	public Rigidbody rb;
	
	public float moveSpeed = 1f;



	// Use this for initialization
	void Start () {
		Vector3 pos = new Vector3 (15, 15, 0);
		rb = GetComponent<Rigidbody> ();
		this.gameObject.name = "soldier";
		transform.Translate(pos * Time.deltaTime);
	}
	
	// Update is called once per frame
	void Update () {
		
	}
}
