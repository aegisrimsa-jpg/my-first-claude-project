// ===== 狀態 =====
let expression = "";

const display = document.getElementById("display");
const stepsEl = document.getElementById("steps");

// 運算符對應中文
const opNames = {
  "+": "加上",
  "-": "減去",
  "*": "乘以",
  "/": "除以",
};

// 運算符顯示符號（給螢幕用）
const opDisplay = {
  "+": " + ",
  "-": " − ",
  "*": " × ",
  "/": " ÷ ",
};

// ===== 輸入字元 =====
function inputChar(ch) {
  if (expression === "" && ch === "0") {
    expression = "0";
  } else {
    expression += ch;
  }
  updateDisplay();
}

// ===== 清除 =====
function clearAll() {
  expression = "";
  display.textContent = "0";
  stepsEl.innerHTML = "";
}

// ===== 更新顯示幕 =====
function updateDisplay() {
  // 將內部符號轉成友善顯示
  let text = expression
    .replace(/\*/g, "×")
    .replace(/\//g, "÷")
    .replace(/-/g, "−");
  display.textContent = text || "0";
}

// ===== 計算 =====
function calculate() {
  if (expression === "") return;

  stepsEl.innerHTML = "";

  // 嘗試計算
  let result;
  try {
    // 安全性檢查：只允許數字、運算符、括號、小數點、空白
    if (!/^[\d+\-*/().  ]+$/.test(expression)) {
      throw new Error("含有不允許的字元");
    }
    result = Function('"use strict"; return (' + expression + ")")();
    if (!isFinite(result)) {
      throw new Error("計算錯誤");
    }
  } catch {
    addStep("算式有誤，請重新輸入！", true);
    return;
  }

  // 產生計算過程
  generateSteps(expression, result);

  // 將結果放回顯示幕，方便繼續運算
  expression = String(result);
  updateDisplay();
}

// ===== 產生步驟說明 =====
function generateSteps(expr, finalResult) {
  // 將算式拆解成 token（數字、運算符、括號）
  const tokens = tokenize(expr);

  let stepNum = 1;

  // 如果算式只有一個數字
  if (tokens.length === 1) {
    addStep("第" + stepNum + "步：取數字 " + tokens[0]);
    stepNum++;
    addStep(
      "第" + stepNum + "步：結果就是 " + formatResult(finalResult),
      true
    );
    return;
  }

  // 一般情況：逐步描述 token
  for (let i = 0; i < tokens.length; i++) {
    const token = tokens[i];

    if (isNumber(token)) {
      if (i === 0 || tokens[i - 1] === "(") {
        addStep("第" + stepNum + "步：取數字 " + token);
        stepNum++;
      }
      // 如果前一個是運算符，已在運算符步驟處理
    } else if (opNames[token]) {
      // 運算符 + 下一個數字一起顯示
      const next = findNextOperand(tokens, i + 1);
      addStep("第" + stepNum + "步：" + opNames[token] + "數字 " + next);
      stepNum++;
    } else if (token === "(") {
      addStep("第" + stepNum + "步：開始括號運算");
      stepNum++;
    } else if (token === ")") {
      addStep("第" + stepNum + "步：結束括號運算");
      stepNum++;
    }
  }

  // 最後一步：顯示結果
  const prettyExpr = formatExpression(expr);
  addStep(
    "第" + stepNum + "步：計算結果 " + prettyExpr + " = " + formatResult(finalResult),
    true
  );
}

// ===== 工具函式 =====

// 拆解 token
function tokenize(expr) {
  const result = [];
  let current = "";
  for (let i = 0; i < expr.length; i++) {
    const ch = expr[i];
    if ("0123456789.".includes(ch)) {
      current += ch;
    } else {
      if (current !== "") {
        result.push(current);
        current = "";
      }
      if ("+-*/()".includes(ch)) {
        result.push(ch);
      }
    }
  }
  if (current !== "") {
    result.push(current);
  }
  return result;
}

// 判斷是否為數字 token
function isNumber(token) {
  return /^[\d.]+$/.test(token);
}

// 找下一個運算元（可能是括號表達式，簡化版只取下一個數字）
function findNextOperand(tokens, startIdx) {
  for (let i = startIdx; i < tokens.length; i++) {
    if (isNumber(tokens[i])) return tokens[i];
  }
  return "?";
}

// 格式化算式顯示
function formatExpression(expr) {
  return expr
    .replace(/\*/g, " × ")
    .replace(/\//g, " ÷ ")
    .replace(/\+/g, " + ")
    .replace(/-/g, " − ")
    .replace(/\(/g, "( ")
    .replace(/\)/g, " )")
    .replace(/  +/g, " ")
    .trim();
}

// 格式化結果（避免過長小數）
function formatResult(num) {
  if (Number.isInteger(num)) return String(num);
  return parseFloat(num.toFixed(8)).toString();
}

// 加入一個步驟到面板
function addStep(text, isResult) {
  const div = document.createElement("div");
  div.className = "step-item" + (isResult ? " result" : "");
  div.textContent = text;
  stepsEl.appendChild(div);
  // 自動捲到最下方
  stepsEl.scrollTop = stepsEl.scrollHeight;
}
