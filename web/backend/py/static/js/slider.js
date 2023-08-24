const dataSlider = document.getElementById('dataSlider');
const sliderValueSpan = document.getElementById('sliderValue');

dataSlider.addEventListener('input', () => {
  const sliderValue = dataSlider.value;
  sliderValueSpan.textContent = sliderValue;
});

dataSlider.addEventListener('change', async () => {
  const sliderValue = dataSlider.value;
  // data = "intensity";
  const requestData = {intensity : parseInt(sliderValue)};
  try {
    const response = await fetch('/api/status', {
      method : 'POST',
      headers : {'Content-Type' : 'application/json'},
      body : JSON.stringify(requestData)
    });
    if (response.ok) {
      console.log("Data sent successfully");
      const responseData = await response.json();
      console.log("Response: ", responseData)
    } else {
      console.error("Failed to send data");
    }
  } catch (error) {
    console.error("An error occurred:", error)
  }
});
