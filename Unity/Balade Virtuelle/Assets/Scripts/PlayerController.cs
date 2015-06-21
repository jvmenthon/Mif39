using UnityEngine;
using System.Collections;

public class PlayerController : MonoBehaviour {

	LineRenderer line;
	AudioSource[] sounds;
	AudioSource laser;
	AudioSource capture;
	AudioSource mouvement;
	AudioSource explosion;
	private float defaultSpeed = 75.0f;
	private float speedForce = 5.0f;
	public float speed;
	private bool superSpeed = false;
	private float lift = 10.0f;
	public float force;
	public ParticleSystem explosionEffect;
	public ParticleSystem reactorEffect;
	public Camera cam;

	void Start()
	{
		line = gameObject.GetComponent<LineRenderer> ();
		sounds = gameObject.GetComponents<AudioSource> ();
		laser = sounds[0];
		capture = sounds[1];
		mouvement = sounds[2];
		explosion = sounds[3];
		line.enabled = false;
		explosionEffect.gameObject.SetActive(false);
		reactorEffect.gameObject.SetActive(false);
		reactorEffect.transform.rotation.Set (0.0f, 0.0f, 0.0f, 0.0f);
		//reactorEffect.transform.rotation.x = 0.0f;
	}


	void Update()
	{
		if (Input.GetButton ("Fire1")) 
		{
			StopCoroutine("FireLaser");
			StartCoroutine("FireLaser");

		}
		if (Input.GetButton ("Fire2")) 
		{
			StopCoroutine("CaptureRay");
			StartCoroutine("CaptureRay");
		}
	}

	void FixedUpdate()
	{
		float CPdistance = Mathf.Sqrt ((transform.position.x - cam.transform.position.x) * (transform.position.x - cam.transform.position.x) +
			(transform.position.y - cam.transform.position.y) * (transform.position.y - cam.transform.position.y) +
			(transform.position.z - cam.transform.position.z) * (transform.position.z - cam.transform.position.z));

		if (Input.GetKey (KeyCode.Z)) {
			reactorEffect.gameObject.transform.position = transform.position;
			transform.position = new Vector3 (transform.position.x + speed * Time.deltaTime * (transform.position.x - cam.transform.position.x) / CPdistance, 
			                                  transform.position.y, 
			                                  transform.position.z + speed * Time.deltaTime * (transform.position.z - cam.transform.position.z) / CPdistance);
			if (!mouvement.isPlaying)
				mouvement.Play ();


			reactorEffect.gameObject.transform.rotation.Set (0.0f, 180.0f, 0.0f, 0.0f);
			reactorEffect.gameObject.SetActive(true);
			reactorEffect.Play(true);
		}
		if(Input.GetKey (KeyCode.S)){
			reactorEffect.gameObject.transform.position = transform.position;
			transform.position = new Vector3 (transform.position.x - speed*Time.deltaTime * (transform.position.x - cam.transform.position.x)/CPdistance, 
			                                  transform.position.y, 
			                                  transform.position.z - speed*Time.deltaTime * (transform.position.z - cam.transform.position.z)/CPdistance);
			if (!mouvement.isPlaying)
				mouvement.Play ();


			reactorEffect.gameObject.transform.rotation.Set (0.0f, 0.0f, 0.0f, 0.0f);
			reactorEffect.gameObject.SetActive(true);
			reactorEffect.Play(true);
		}
		if (Input.GetKey (KeyCode.D)){
			reactorEffect.gameObject.transform.position = transform.position;
			transform.position = new Vector3 (transform.position.x + speed*Time.deltaTime * (transform.position.z - cam.transform.position.z)/CPdistance, 
			                                  transform.position.y, 
			                                  transform.position.z - speed*Time.deltaTime * (transform.position.x - cam.transform.position.x)/CPdistance);
			if (!mouvement.isPlaying)
				mouvement.Play ();


			reactorEffect.gameObject.transform.rotation.Set (0.0f, 90.0f, 0.0f, 0.0f);
			reactorEffect.gameObject.SetActive(true);
			reactorEffect.Play(true);
		}
		if(Input.GetKey (KeyCode.Q)){
			reactorEffect.gameObject.transform.position = transform.position;
			transform.position = new Vector3 (transform.position.x - speed*Time.deltaTime * (transform.position.z - cam.transform.position.z)/CPdistance, 
			                                  transform.position.y, 
			                                  transform.position.z + speed*Time.deltaTime * (transform.position.x - cam.transform.position.x)/CPdistance);
			if (!mouvement.isPlaying)
				mouvement.Play ();

			reactorEffect.gameObject.transform.rotation.Set (0.0f, 270.0f, 0.0f, 0.0f);
			reactorEffect.gameObject.SetActive(true);
			reactorEffect.Play(true);
		}	

		if (Input.GetKey (KeyCode.A)){
			reactorEffect.gameObject.transform.position = transform.position;
			transform.position = new Vector3 (transform.position.x, transform.position.y + lift*Time.deltaTime, transform.position.z);
			if (!mouvement.isPlaying)
				mouvement.Play ();

			reactorEffect.gameObject.transform.rotation.Set (90.0f, 0.0f, 0.0f, 0.0f);
			reactorEffect.gameObject.SetActive(true);
			reactorEffect.Play(true);
		}
		if(Input.GetKey (KeyCode.E)){
			reactorEffect.gameObject.transform.position = transform.position;
			transform.position = new Vector3 (transform.position.x, transform.position.y - lift*Time.deltaTime, transform.position.z);
			if (!mouvement.isPlaying)
				mouvement.Play ();

			reactorEffect.gameObject.transform.rotation.Set (-90.0f, 0.0f, 0.0f, 0.0f);
			reactorEffect.gameObject.SetActive(true);
			reactorEffect.Play(true);
		}
		if (Input.GetKeyDown (KeyCode.LeftShift)) 
		{
			if(superSpeed == false)
			{
				speed = defaultSpeed * speedForce;
				superSpeed = true;
			}

			else
				speed = defaultSpeed;
		}
	}

	IEnumerator FireLaser()
	{
		Vector4 redLaser = new Vector4 (1.0f, 0.0f, 0.0f, 0.5f);
		line.SetColors (redLaser,redLaser);
		line.SetWidth (1.0f, 1.0f);
		line.enabled = true;
		if(!laser.isPlaying)
			laser.Play();



		while (Input.GetButtonDown("Fire1")) 
		{
			//Ray ray = new Ray(transform.position,transform.forward);
			Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
			RaycastHit hit;

			line.SetPosition(0,transform.position);

			if(Physics.Raycast(ray,out hit, 500) && hit.rigidbody != gameObject.GetComponent<Rigidbody>())
			{
			    line.SetPosition(1,hit.point);
				hit.rigidbody.AddExplosionForce(force,hit.point,1,0,ForceMode.Impulse);
				hit.collider.gameObject.SetActive(false);
				explosionEffect.gameObject.SetActive(true);
				explosionEffect.gameObject.transform.position = hit.point;
				explosionEffect.Play(true);
				laser.Stop();
				explosion.Play();
			}
				
			else
			{
			   line.SetPosition(1,hit.point);
			}

			yield return null;
		}
		line.enabled = false;

	}

	IEnumerator CaptureRay()
	{
		Vector4 greenLaser = new Vector4 (0.0f, 1.0f, 0.0f, 0.5f);
		line.SetColors (greenLaser,greenLaser);
		line.SetWidth (5.0f, 10.0f);
		line.enabled = true;
		if(!capture.isPlaying)
			capture.Play();
		while (Input.GetButton("Fire2")) 
		{

			//Ray ray = new Ray(transform.position,transform.forward);
			Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
			RaycastHit hit;
			
			line.SetPosition(0,transform.position);
			
			if(Physics.Raycast(ray,out hit, 500) && hit.rigidbody != gameObject.GetComponent<Rigidbody>())
			{
				line.SetPosition(1,hit.point);
				Vector3 way = hit.point - transform.position;
				hit.rigidbody.AddForce(-force * way,ForceMode.Force);
			}
			
			else
			{
				line.SetPosition(1,hit.point);
			}
			
			yield return null;
		}
		line.enabled = false;


	}
}
