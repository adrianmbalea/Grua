#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
in float transparencia;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform sampler2D texture1;

void main()

{
	vec3 luzDir = vec3(0,0,-1);

	//ambiente
	float ambientI = 1.0f;
	vec3 ambient = ambientI * lightColor;

	vec3 ld = normalize(-lightPos);
	ld = luzDir;
	vec3 fd = normalize(vec3((FragPos-lightPos)));

	if(acos(dot(fd,ld)) < radians(3.0)){
		//diffuse
		vec3 norm = normalize(Normal);
		vec3 lightDir = normalize(lightPos - FragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * lightColor;

		//specular
		float specularStrength = 1.0;
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
		vec3 specular = specularStrength * spec * lightColor;

		vec3 result = ((ambient + diffuse + specular) * objectColor)/2;
		FragColor = vec4(result, 0.75) * texture2D(texture1, TexCoord);
	}

	else{
		vec3 result = ((ambient) * objectColor)/2;							// Calcula luz de ambiente
		FragColor = vec4(result, 0.75) *  texture2D(texture1, TexCoord);
	}

}