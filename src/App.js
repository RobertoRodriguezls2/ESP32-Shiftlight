import React, { Component } from 'react';
import ToggleButton from 'react-toggle-button';
import logo from './logo.svg';
import './App.css';

class App extends Component {
  constructor(props) {
    super(props);
    this.state = {
      ledOn: false,
      value: '',
      value2: '',
      value3: '',
      value4: '',
      lowEnd: '12345',
      midRange: '',
      highEnd: '',
      testRPM: ''


    };

    this.handleChangeLOW = this.handleChangeLOW.bind(this);
    this.handleSubmitLOW = this.handleSubmitLOW.bind(this);

    this.handleChangeMID = this.handleChangeMID.bind(this);
    this.handleSubmitMID = this.handleSubmitMID.bind(this);

    this.handleChangeHIGH = this.handleChangeHIGH.bind(this);
    this.handleSubmitHIGH = this.handleSubmitHIGH.bind(this);

    this.handleChangeRPM = this.handleChangeRPM.bind(this);
    this.handleSubmitRPM = this.handleSubmitRPM.bind(this);

  }
  handleChangeLOW(event) { this.setState({ value: event.target.value, lowEnd: event.target.value }); }
  handleChangeMID(event) { this.setState({ value2: event.target.value, midRange: event.target.value }); }
  handleChangeHIGH(event) { this.setState({ value3: event.target.value, highEnd: event.target.value }); }
  handleChangeRPM(event) { this.setState({ value4: event.target.value, testRPM: event.target.value }); }

  setMsgState(state) {
    this.setState({ lowEnd: state })
  }

  setMsgState2(state) {
    this.setState({ midRange: state })
  }

  setMsgState3(state) {
    this.setState({ highEnd: state })
  }

  handleSubmitLOW(event) {
    //alert('A number was submitted: ' + this.state.value);
    fetch('/LOW', { method: 'PUT', body: this.state.lowEnd })
      .then((response) => response.json())
      .then(response => response.text())
      .then(data => this.setState({}));
    event.preventDefault();
  }

  handleSubmitMID(event) {
    //alert('A number was submitted: ' + this.state.value2);
    fetch('/MID', { method: 'PUT', body: this.state.midRange })
      .then((response) => response.json())
      .then(response => response.text())
      .then(data => this.setState({}));
    event.preventDefault();
  }

  handleSubmitHIGH(event) {
    //alert('A number was submitted: ' + this.state.value);
    fetch('/HIGH', { method: 'PUT', body: this.state.highEnd })
      .then((response) => response.json())
      .then(response => response.text())
      .then(data => this.setState({}));
    event.preventDefault();
  }

  handleSubmitRPM(event) {
    //alert('A number was submitted: ' + this.state.value);
    fetch('/TEST', { method: 'PUT', body: this.state.testRPM })
      .then((response) => response.json())
      .then(response => response.text())
      .then(data => this.setState({}));
    event.preventDefault();
  }

  setLedState(state) {
    this.setState({ ledOn: state !== '0' })
  }

  componentDidMount() {
    fetch('/led')
      .then(response => response.text())
      .then(state => this.setLedState(state));
  }

  handleStateChange(ledOn) {
    fetch('/led', { method: 'PUT', body: ledOn ? '0' : '1' })
      .then(response => response.text())
      .then(state => this.setLedState(state));
  }

  handleStrip(lights) {
    fetch('/strip', { method: 'GET' })
      .then(response => response.json())
      .then(data => console.log(data));

  }

  render() {
    return (
      <div className="App">
        <header className="App-header">
          {/* <img src={logo} className="App-logo" alt="logo" />
          <ToggleButton
            value={this.state.ledOn}
            onToggle={value => this.handleStateChange(value)}
          /> */}

          <div className='row'>

            <form className='column' value={this.state.lowEnd} onSubmit={value => this.handleSubmitLOW(value)}>
              <label> Low End
                <br />
                <label />
                <input type="number"
                  value={this.state.value} onChange={this.handleChangeLOW} />        </label>
              <input type="submit" value="Set" />
            </form>

            <form className='column' value={this.state.midRange} onSubmit={value => this.handleSubmitMID(value)}>
              <label>
                Mid Range
                <br />
                <label />
                <input type="number"
                  value={this.state.value2} onChange={this.handleChangeMID} />        </label>
              <input type="submit" value="Set" />
            </form>

            <form className='column' value={this.state.highEnd} onSubmit={value => this.handleSubmitHIGH(value)}>
              <label>
              High End
              <br/>
              <label/>
              <input type="number"
                value={this.state.highEnd} onChange={this.handleChangeHIGH} />        </label>
              <input type="submit" value="Set" />
            </form>

            <form className='column' value={this.state.testRPM} onSubmit={value => this.handleSubmitRPM(value)}>
              <label>
              Test RPM
              <br/>
              <label/>
              <input type="number"
                value={this.state.testRPM} onChange={this.handleChangeRPM} />        </label>
              <input type="submit" value="Set" />
            </form>

          </div>


        </header>
      </div>
    );
  }
}

export default App;
