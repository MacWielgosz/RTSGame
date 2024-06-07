#version 330 core

// Input vertex attributes (from vertex shader)
in vec3 fragNormal;
in vec3 fragPosition;

// Output fragment color
out vec4 finalColor;

// Input uniform values
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 viewPosition;
uniform vec4 modelColor;

void main()
{
    // Normalize the incoming normal
    vec3 normal = normalize(fragNormal);
    
    // Calculate the light direction vector
    vec3 lightDir = normalize(lightPosition - fragPosition);
    
    // Calculate the view direction vector
    vec3 viewDir = normalize(viewPosition - fragPosition);
    
    // Calculate the reflection direction vector
    vec3 reflectDir = reflect(-lightDir, normal);
    
    // Ambient component
    vec3 ambient = 0.1 * lightColor;
    
    // Diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular component
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * lightColor;
    
    // Combine the results
    vec3 result = (ambient + diffuse + specular) * modelColor.rgb;
    finalColor = vec4(result, modelColor.a);
}
