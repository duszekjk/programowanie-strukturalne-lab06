using UnityEngine;

namespace IpcCharacterWorld
{
    [RequireComponent(typeof(CharacterController))]
    public sealed class CommandDrivenCharacter : MonoBehaviour
    {
        public float stepDistance = 1.0f;
        public float turnDegrees = 45.0f;
        public float jumpImpulse = 6.0f;
        public float gravity = -20.0f;

        private CharacterController controller;
        private Vector3 startPosition;
        private Quaternion startRotation;
        private float verticalSpeed;

        private void Awake()
        {
            controller = GetComponent<CharacterController>();
            startPosition = transform.position;
            startRotation = transform.rotation;
        }

        private void Update()
        {
            if (controller.isGrounded && verticalSpeed < 0.0f)
            {
                verticalSpeed = -1.0f;
            }

            verticalSpeed += gravity * Time.deltaTime;
            controller.Move(Vector3.up * verticalSpeed * Time.deltaTime);
        }

        public void Execute(CharacterCommand command)
        {
            if (command == CharacterCommand.Forward)
            {
                controller.Move(transform.forward * stepDistance);
            }
            else if (command == CharacterCommand.Backward)
            {
                controller.Move(-transform.forward * stepDistance);
            }
            else if (command == CharacterCommand.Left)
            {
                transform.Rotate(0.0f, -turnDegrees, 0.0f);
            }
            else if (command == CharacterCommand.Right)
            {
                transform.Rotate(0.0f, turnDegrees, 0.0f);
            }
            else if (command == CharacterCommand.Jump)
            {
                if (controller.isGrounded)
                {
                    verticalSpeed = jumpImpulse;
                }
            }
            else if (command == CharacterCommand.Reset)
            {
                controller.enabled = false;
                transform.SetPositionAndRotation(startPosition, startRotation);
                controller.enabled = true;
                verticalSpeed = 0.0f;
            }
        }
    }
}
