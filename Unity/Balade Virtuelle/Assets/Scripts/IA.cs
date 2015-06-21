using UnityEngine;
using System.Collections;
using System;

public class IA : MonoBehaviour
{
	public GameObject target;
	private GameObject ammo;
	private int shootFrequency = 2;
	private float turnSpeed = 3.0f;
	private Vector3 rotate;
	private float force = 4.0f;
	LineRenderer line;
	// Use this for initialization
	void Start ()
	{
		//ammo = Instantiate(Resources.Load("balle", typeof(GameObject))) as GameObject;
		rotate = transform.position - target.transform.position;
		line = gameObject.GetComponent<LineRenderer> ();
		line.enabled = false;
	}
	
	// Update is called once per frame
	void Update ()
	{
		if (Time.time % shootFrequency == 0) 
		{
			StopCoroutine ("shoot");
			StartCoroutine ("shoot");
		}
	}

	void FixedUpdate()
	{
		Vector3 dir = target.transform.position - transform.position;
		dir = target.transform.InverseTransformDirection(dir);
		float angle = Mathf.Atan2(dir.y, dir.x) * Mathf.Rad2Deg;
		rotate = Quaternion.AngleAxis (angle * turnSpeed, Vector3.up) * rotate;
		transform.rotation.Set(rotate.x,rotate.y,rotate.z,0.0f);

		/*
		 * 	var rotation = Quaternion.LookRotation(cible.position);
		 	rotation *= Quaternion.Euler(0, 0, -90);
		 	transform.rotation = rotation;
			
			var temps : int = Time.time;
			var cadence = (temps % 4);
			if(cadence) {
				tir(temps);
			}
	*/
	}
	//
	IEnumerator shoot()
	{
		line.enabled = true;
		while (Time.time % shootFrequency == 0) 
		{
			//ammo = Instantiate(Resources.Load("balle", typeof(GameObject))) as GameObject;
			Vector3 way = transform.position - target.transform.position;
			//ammo.GetComponent<Rigidbody>().AddForce(way * 1000);
			Ray ray = new Ray(transform.position,transform.forward);
			RaycastHit hit;

			line.SetPosition (0, transform.position);
			
			if (Physics.Raycast (ray, out hit, 500) && hit.rigidbody != gameObject.GetComponent<Rigidbody> ()) {
				line.SetPosition (1, hit.point);
				hit.rigidbody.AddExplosionForce (force, hit.point, 1, 0, ForceMode.Impulse);
				//hit.collider.gameObject.SetActive (false);
			} else {
				line.SetPosition (1, hit.point);
			}
			yield return null;
		}

		line.enabled = false;
	}
}

