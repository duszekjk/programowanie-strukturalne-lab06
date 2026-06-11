using System;

namespace IpcCharacterWorld
{
    public static class CommandParser
    {
        public static bool TryParse(string text, out CharacterCommand command)
        {
            command = CharacterCommand.Stop;
            if (string.IsNullOrWhiteSpace(text))
            {
                return false;
            }

            string value = text.Trim().ToUpperInvariant();

            switch (value)
            {
                case "FORWARD":
                case "F":
                    command = CharacterCommand.Forward;
                    return true;
                case "BACKWARD":
                case "BACK":
                case "B":
                    command = CharacterCommand.Backward;
                    return true;
                case "LEFT":
                case "L":
                    command = CharacterCommand.Left;
                    return true;
                case "RIGHT":
                case "R":
                    command = CharacterCommand.Right;
                    return true;
                case "JUMP":
                case "J":
                    command = CharacterCommand.Jump;
                    return true;
                case "STOP":
                case "S":
                    command = CharacterCommand.Stop;
                    return true;
                case "RESET":
                    command = CharacterCommand.Reset;
                    return true;
                default:
                    return false;
            }
        }
    }
}
