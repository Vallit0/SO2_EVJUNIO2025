def xor_encrypt_decrypt(input_string: str, key: str) -> str:
    result = list(input_string)
    
    # Perform XOR on each character in the input string
    for i in range(len(input_string)):
        
        # XOR the character with the corresponding character in the key
    
        # - The ord() function takes a character as input and returns its corresponding Unicode code point. In other words, it converts the character to its integer representation.
        # - key[i % len(key)]: This retrieves the character at index i % len(key) from the key string. 
        #     The %  ensures that the key is repeated cyclically if its length is shorter than the input string.
        # - The ^ operator performs a bitwise XOR operation between the two Unicode values.
        #     XOR (exclusive OR) compares the binary representations of the code points and returns a new value that represents the result of the XOR operation.
        
        result[i] = chr(ord(input_string[i]) ^ ord(key[i % len(key)]))
        
    # Return the encrypted/decrypted string
    return ''.join(result)

# Example usage
if __name__ == "__main__":
    text = "Sistemas Operativos 2 2S2024"
    key = "key123"

    print(f"Original Text: {text}")

    # Encrypt the text
    encrypted_text = xor_encrypt_decrypt(text, key)
    print(f"Encrypted Text: {encrypted_text}")

    # Decrypt the text (since XOR encryption is symmetric, applying the same function will decrypt it)
    decrypted_text = xor_encrypt_decrypt(encrypted_text, key)
    print(f"Decrypted Text: {decrypted_text}")