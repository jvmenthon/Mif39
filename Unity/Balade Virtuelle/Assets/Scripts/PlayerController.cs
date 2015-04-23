using UnityEngine;
using System.Collections;

public class PlayerController : MonoBehaviour {

	LineRenderer line;
	private float defaultSpeed = 25.0f;
	public float speed;
	private float lift = 10.0f;
	public float force;

	void Start()
	{
		line = gameObject.GetComponent<LineRenderer> ();
		line.enabled = false;
	}


	void Update()
	{
		if (Input.GetButtonDown ("Fire1")) 
		{
			StopCoroutine("FireLaser");
			StartCoroutine("FireLaser");
		}
	}

	void FixedUpdate()
	{

		if (Input.GetKey (KeyCode.Z))
			transform.position = new Vector3 (transform.position.x, transform.position.y, transform.position.z + speed*Time.deltaTime);
		
		if(Input.GetKey (KeyCode.S))
			transform.position = new Vector3 (transform.position.x, transform.position.y, transform.position.z - speed*Time.deltaTime);

		if (Input.GetKey (KeyCode.D))
			transform.position = new Vector3 (transform.position.x + speed*Time.deltaTime, transform.position.y, transform.position.z);
		
		if(Input.GetKey (KeyCode.Q))
			transform.position = new Vector3 (transform.position.x - speed*Time.deltaTime, transform.position.y, transform.position.z);

		if (Input.GetKey (KeyCode.A))
			transform.position = new Vector3 (transform.position.x, transform.position.y + lift*Time.deltaTime, transform.position.z);

		if(Input.GetKey (KeyCode.E))
			transform.position = new Vector3 (transform.position.x, transform.position.y - lift*Time.deltaTime, transform.position.z);

		if (Input.GetKey (KeyCode.LeftShift))
			speed = defaultSpeed * 2;
	}

	IEnumerator FireLaser()
	{
		line.enabled = true;

		while (Input.GetButton("Fire1")) 
		{
			//Ray ray = new Ray(transform.position,transform.forward);
			Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
			RaycastHit hit;

			line.SetPosition(0,transform.position);

			if(Physics.Raycast(ray,out hit, 500))
			{
			   line.SetPosition(1,hit.point);
				hit.rigidbody.AddExplosionForce(force,hit.point,5,0,ForceMode.Impulse);
			}
				
			else
			   line.SetPosition(1,ray.GetPoint(500));

			yield return null;
		}
		line.enabled = false;
	}
}
